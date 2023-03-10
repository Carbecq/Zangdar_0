#ifndef LIBCHESS_POSITION_HPP
#define LIBCHESS_POSITION_HPP

#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "bitboard.h"
#include "move.h"
#include "piece.h"
#include "color.h"
#include "zobrist.h"
#include "defines.h"
#include "MoveList.h"



typedef struct Mask {
int direction[64];
} Mask;


// structure destinée à stocker l'historique de make_move.
// celle-ci sera nécessaire pour effectuer un unmake_move
typedef struct UndoInfo {
    U64         hash = 0;               // nombre unique (?) correspondant à la position
    U32         move;
    int         ep_square;              // case en-passant : si les blancs jouent e2-e4, la case est e3
    int         halfmove_clock = 0;     // nombre de coups depuis une capture, ou un movement de pion
    U32         castling;               // droit au roque
} UndoInfo;

/*******************************************************
 ** Droit au roque
 **---------------------------------------------------*/
enum Castle {
    CASTLE_NONE = 0,
    CASTLE_WK   = 1,
    CASTLE_WQ   = 2,
    CASTLE_BK   = 4,
    CASTLE_BQ   = 8
};

constexpr const int ksc_castle_rook_from[]    = {H1, H8};
constexpr const int ksc_castle_rook_to[]      = {F1, F8};

constexpr const int qsc_castle_rook_from[]    = {A1, A8};
constexpr const int qsc_castle_rook_to[]      = {D1, D8};

constexpr const int castle_king_from[]   = {E1, E8};
constexpr const int ksc_castle_king_to[] = {G1, G8};
constexpr const int qsc_castle_king_to[] = {C1, C8};

// }  // namespace

class Board
{
public:
    [[nodiscard]] Board() noexcept;
    [[nodiscard]] Board(const std::string &fen) noexcept;

    void position(std::istringstream &is);

    [[nodiscard]] constexpr Color     turn() const noexcept { return side_to_move; }

    template<Color C>              [[nodiscard]] constexpr Bitboard occupancy_c() const noexcept { return colorPiecesBB[C]; }
    template<PieceType P>          [[nodiscard]] constexpr uint64_t occupancy_p() const noexcept { return typePiecesBB[P]; }
    template<Color C, PieceType P> [[nodiscard]] constexpr Bitboard pieces_cp()   const noexcept { return colorPiecesBB[C] & typePiecesBB[P]; }

    //Returns the bitboard of all bishops and queens of a given color
    template<Color C>
    constexpr Bitboard diagonal_sliders() const {
        return C == WHITE ? pieces_cp<WHITE, PieceType::Bishop>() | pieces_cp<WHITE, PieceType::Queen>() :
                            pieces_cp<BLACK, PieceType::Bishop>() | pieces_cp<BLACK, PieceType::Queen>();
    }

    //Returns the bitboard of all rooks and queens of a given color
    template<Color C>
    constexpr Bitboard orthogonal_sliders() const {
        return C == WHITE ? pieces_cp<WHITE, PieceType::Rook>() | pieces_cp<WHITE, PieceType::Queen>() :
                            pieces_cp<BLACK, PieceType::Rook>() | pieces_cp<BLACK, PieceType::Queen>();
    }

    [[nodiscard]] constexpr Bitboard occupied()     const noexcept { return colorPiecesBB[WHITE] | colorPiecesBB[BLACK]; }
    [[nodiscard]] constexpr Bitboard non_occupied() const noexcept { return ~occupied(); }

    template<Color C>
    constexpr bool major_pieces() const  {
        return (Bcount(colorPiecesBB[C] ^ pieces_cp<C, PieceType::Pawn>() ^ pieces_cp<C, PieceType::King>()) > 0);

    }
    void set_fen(const std::string &fen, bool logTactics) noexcept;
    [[nodiscard]] std::string get_fen() const noexcept;
    void mirror_fen(const std::string& fen, bool logTactics);

    void test_rays();



    [[nodiscard]] constexpr int get_halfmove_clock() const noexcept { return halfmove_clock; }
    [[nodiscard]] constexpr int get_game_clock() const noexcept { return game_clock; }
    [[nodiscard]] constexpr std::size_t get_fullmove_clock() const noexcept { return fullmove_clock; }

    template <Color C> [[nodiscard]] constexpr int      king_position() const noexcept { return x_king[C]; /*return first_square( pieces_cp<C, PieceType::King>() ); */}
    template <Color C> [[nodiscard]] constexpr Bitboard squares_attacked() const noexcept;
    template <Color C> [[nodiscard]] constexpr bool     square_attacked(const int sq) const noexcept { return attackers<C>(sq) > 0; }
    template <Color C> [[nodiscard]] constexpr Bitboard checkers() const noexcept { return attackers<C>(king_position<C>()); }
    template <Color C> [[nodiscard]] constexpr Bitboard attackers(const int sq) const noexcept;

    /* check if an enpassant move is possible */
     [[nodiscard]] constexpr bool board_enpassant() { return ep_square != NO_SQUARE; }

    template <Color C> constexpr void legal_moves(MoveList& ml)  noexcept;
    template <Color C> constexpr void legal_captures(MoveList& ml) noexcept;
    template <Color C> constexpr void legal_evasions(MoveList& ml) noexcept;

    template <Color C> void apply_token(const std::string& token) noexcept;

    void verify_MvvLva();

    void add_quiet_move       (MoveType type, MoveList& ml, int from, int dest, PieceType piece) const noexcept;
    void add_capture_move     (MoveType type, MoveList& ml, int from, int dest, PieceType piece, PieceType captured) const noexcept;
    void add_quiet_promotion  (MoveType type, MoveList& ml, int from, int dest, PieceType promo) const noexcept;
    void add_capture_promotion(MoveType type, MoveList& ml, int from, int dest, PieceType captured, PieceType promo) const noexcept;

    void push_quiet_moves(MoveList& ml, Bitboard attack, const int from);
    void push_capture_moves(MoveList& ml, Bitboard attack, const int from);
    void push_piece_quiet_moves(MoveList& ml, Bitboard attack, const int from, PieceType piece);
    void push_piece_capture_moves(MoveList& ml, Bitboard attack, const int from, PieceType piece);
    void push_quiet_promotions(MoveList& ml, Bitboard attack, const int dir);
    void push_capture_promotions(MoveList& ml, Bitboard attack, const int dir);
    void push_quiet_promotion(MoveList& ml, const int from, const int to);
    void push_capture_promotion(MoveList& ml, const int from, const int to);
    void push_pawn_quiet_moves(MoveType type, MoveList& ml, Bitboard attack, const int dir);
    void push_pawn_capture_moves(MoveList& ml, Bitboard attack, const int dir);

    template <Color C> void generate_checkers();

    template <Color C> [[nodiscard]] std::uint64_t perft(const int depth) noexcept;
    template <Color C> [[nodiscard]] std::uint64_t divide(const int depth) noexcept;

    template <Color C> [[nodiscard]] constexpr bool can_castle() const noexcept
    {
        switch (C)
        {
        case WHITE:
            return castling & (CASTLE_WK | CASTLE_WQ);
            break;
        case BLACK:
            return castling & (CASTLE_BK | CASTLE_BQ);
            break;
        default:
            return false;
        }
    }

    template <Color C> [[nodiscard]] constexpr bool can_castle_k() const noexcept
    {
        switch (C)
        {
        case WHITE:
            return castling & CASTLE_WK;
            break;
        case BLACK:
            return castling & CASTLE_BK;
            break;
        default:
            return false;
        }
    }

    template <Color C> [[nodiscard]] constexpr bool can_castle_q() const noexcept
    {
        switch (C)
        {
        case WHITE:
            return castling & CASTLE_WQ;
            break;
        case BLACK:
            return castling & CASTLE_BQ;
            break;
        default:
            return false;
        }
    }

    [[nodiscard]] constexpr bool white_can_castle_k() const noexcept  { return castling & CASTLE_WK; }
    [[nodiscard]] constexpr bool white_can_castle_q() const noexcept  { return castling & CASTLE_WQ; }
    [[nodiscard]] constexpr bool black_can_castle_k() const noexcept  { return castling & CASTLE_BK; }
    [[nodiscard]] constexpr bool black_can_castle_q() const noexcept  { return castling & CASTLE_BQ; }

    template <Color C> constexpr void make_move(const U32 move) noexcept;
    template <Color C> constexpr void undo_move() noexcept;
    template <Color C> constexpr void make_nullmove() noexcept;
    template <Color C> constexpr void undo_nullmove() noexcept;


    [[nodiscard]] constexpr U64 calculate_hash() const noexcept
    {
        U64 khash = 0ULL;
        Bitboard bb;

        // Turn
        if (turn() == Color::BLACK) {
            khash ^= side_key;
        }

        // Pieces
            bb = pieces_cp<WHITE, PieceType::Pawn>();
            while (bb) {
                int sq = next_square(bb);
                khash ^= piece_key[WHITE][PieceType::Pawn][sq];
            }
            bb = pieces_cp<WHITE, PieceType::Knight>();
            while (bb) {
                int sq = next_square(bb);
                khash ^= piece_key[WHITE][PieceType::Knight][sq];
            }
            bb = pieces_cp<WHITE, PieceType::Bishop>();
            while (bb) {
                int sq = next_square(bb);
                khash ^= piece_key[WHITE][PieceType::Bishop][sq];
            }
            bb = pieces_cp<WHITE, PieceType::Rook>();
            while (bb) {
                int sq = next_square(bb);
                khash ^= piece_key[WHITE][PieceType::Rook][sq];
            }
            bb = pieces_cp<WHITE, PieceType::Queen>();
            while (bb) {
                int sq = next_square(bb);
                khash ^= piece_key[WHITE][PieceType::Queen][sq];
            }
            bb = pieces_cp<WHITE, PieceType::King>();
            while (bb) {
                int sq = next_square(bb);
                khash ^= piece_key[WHITE][PieceType::King][sq];
            }
            bb = pieces_cp<BLACK, PieceType::Pawn>();
            while (bb) {
                int sq = next_square(bb);
                khash ^= piece_key[BLACK][PieceType::Pawn][sq];
            }
            bb = pieces_cp<BLACK, PieceType::Knight>();
            while (bb) {
                int sq = next_square(bb);
                khash ^= piece_key[BLACK][PieceType::Knight][sq];
            }
            bb = pieces_cp<BLACK, PieceType::Bishop>();
            while (bb) {
                int sq = next_square(bb);
                khash ^= piece_key[BLACK][PieceType::Bishop][sq];
            }
            bb = pieces_cp<BLACK, PieceType::Rook>();
            while (bb) {
                int sq = next_square(bb);
                khash ^= piece_key[BLACK][PieceType::Rook][sq];
            }
            bb = pieces_cp<BLACK, PieceType::Queen>();
            while (bb) {
                int sq = next_square(bb);
                khash ^= piece_key[BLACK][PieceType::Queen][sq];
            }
            bb = pieces_cp<BLACK, PieceType::King>();
            while (bb) {
                int sq = next_square(bb);
                khash ^= piece_key[BLACK][PieceType::King][sq];
            }
        // Castling
        khash ^= castle_key[castling];

        // EP
        if (ep_square != NO_SQUARE) {
            khash ^= ep_key[ep_square];
        }

        return khash;
    }

    [[nodiscard]] constexpr PieceType piece_on(const int sq) const noexcept {
        for (int i = 0; i < 6; ++i) {
            if (typePiecesBB[i] & square_to_bit(sq)) {
                return PieceType(i);
            }
        }
        return PieceType::NO_TYPE;
    }

    [[nodiscard]] constexpr int ep() const noexcept { return ep_square; }
    [[nodiscard]] constexpr std::uint64_t get_hash() const noexcept {return hash;  }

    void clear() noexcept
    {
        colorPiecesBB[0] = 0ULL;
        colorPiecesBB[1] = 0ULL;
        typePiecesBB[0] = 0ULL;
        typePiecesBB[1] = 0ULL;
        typePiecesBB[2] = 0ULL;
        typePiecesBB[3] = 0ULL;
        typePiecesBB[4] = 0ULL;
        typePiecesBB[5] = 0ULL;

        cpiece.fill(PieceType::NO_TYPE);

        halfmove_clock  = 0;
        fullmove_clock  = 1;
        game_clock      = 0;
        hash            = 0;

        ep_square    = NO_SQUARE;
        hash         = 0x0;
        castling     = 0;
        side_to_move = Color::WHITE;
        //the_history.clear(); est-ce nécessaire ??

    }

    template <Color C> [[nodiscard]] constexpr bool valid() const noexcept;
    [[nodiscard]] std::string display() const noexcept;
    void pv_move(MoveList& move_list, U32 PvMove);

    //================================== partie UCI
    // Fonction UCI
    //    void position(std::istringstream &is);
    //    void new_game();
    //    void go(std::istringstream &is);
    //    void stop();
    //    void quit();

    //=================================== recherche
    void reset();
    void init();

    //=================================== evaluation
    [[nodiscard]] int evaluate();
    template <Color C> constexpr void evaluate_0(int& mg, int& eg, int& gamePhase);

    [[nodiscard]] bool MaterialDraw() const ;

    void board_deplace_piece(const int from, const int to) ;
    void board_restore(const U32 move) ;
    void init_mask();
    void generate_checkers();
    void init_allmask();
    void init_bitmasks();


    Bitboard  colorPiecesBB[2] = {ZERO};  // occupancy board pour chaque couleur
    Bitboard  typePiecesBB[6]  = {ZERO};   // bitboard pour chaque type de piece


    std::array<UndoInfo, MAX_HIST>   the_history;        // données à conserver pour 'unmake_move'

    //------------------------------------------------------- la position
    Color   side_to_move    = Color::WHITE;    // camp au trait
    int     ep_square       = NO_SQUARE;       // case en-passant : si les blancs jouent e2-e4, la case est e3
    U32     castling        = CASTLE_NONE;      // droit au roque

    int     halfmove_clock  = 0;        // nombre de demi-coups depuis la dernière capture ou le dernier mouvement de pion.
    int     game_clock      = 0;        // nombre de demi-coups de la partie
    int     fullmove_clock  = 1;        // le nombre de coups complets. Il commence à 1 et est incrémenté de 1 après le coup des noirs.

    U64     hash            = 0ULL;     // nombre unique (?) correspondant à la position (clef Zobrist)



    std::vector<std::string>  best_moves;       // meilleur coup (pour les test tactique)
    std::vector<std::string>  avoid_moves;      // coup à éviter (pour les test tactique)

// private:


    template <Color C> [[nodiscard]] constexpr bool is_in_check()  const noexcept { return square_attacked<~C>(king_position<C>()); }

    //====================================================================
    //! \brief  Détermine s'il y a eu 50 coups sans prise ni coup de pion
    //--------------------------------------------------------------------
    [[nodiscard]] constexpr bool fiftymoves() const noexcept { return halfmove_clock >= 100; }

    //====================================================================
    //! \brief  Détermine s'il y a eu répétition de la même position
    //! Pour cela, on compare le hash code de la position.
    //--------------------------------------------------------------------
    [[nodiscard]] bool is_repetition() const noexcept
    {
        for (int i = game_clock - halfmove_clock; i < game_clock; ++i)
        {
            assert(i >= 0 && i < MAX_HIST);

            if (the_history[i].hash == hash)
                return true;
        }
        return false;
    }

    //=============================================================================
    //! \brief  Détermine si la position est nulle
    //-----------------------------------------------------------------------------
    template <Color C> [[nodiscard]] constexpr bool is_draw() const noexcept {
        return ( (is_repetition() || fiftymoves()));
    }







    void set(const int sq, const Color s, const PieceType p) noexcept
    {
        colorPiecesBB[s] |= square_to_bit(sq);
        typePiecesBB[p]  |= square_to_bit(sq);
        cpiece[sq]        = p;
    }

    std::array<PieceType, 64>   cpiece;
    int       x_king[2];                    // position des rois

    Bitboard pinnedBB;
    Bitboard checkersBB;
    std::array<Mask, 64> allmask;





};


//================================================================================
//! \brief  Affichage de l'échiquier
//--------------------------------------------------------------------------------
inline std::ostream &operator<<(std::ostream &os, const Board &pos) noexcept
{
    int i = 56;
    os << std::endl;

    os << "  8 " ;

    while (i >= 0) {
        const auto sq = i;
        const auto bb = square_to_bit(sq);

        if (pos.pieces_cp<Color::WHITE, PieceType::Pawn>() & bb) {
            os << 'P';
        } else if (pos.pieces_cp<Color::WHITE, PieceType::Knight>() & bb) {
            os << 'N';
        } else if (pos.pieces_cp<Color::WHITE, PieceType::Bishop>() & bb) {
            os << 'B';
        } else if (pos.pieces_cp<Color::WHITE, PieceType::Rook>() & bb) {
            os << 'R';
        } else if (pos.pieces_cp<Color::WHITE, PieceType::Queen>() & bb) {
            os << 'Q';
        } else if (pos.pieces_cp<Color::WHITE, PieceType::King>() & bb) {
            os << 'K';
        } else if (pos.pieces_cp<Color::BLACK, PieceType::Pawn>() & bb) {
            os << 'p';
        } else if (pos.pieces_cp<Color::BLACK, PieceType::Knight>() & bb) {
            os << 'n';
        } else if (pos.pieces_cp<Color::BLACK, PieceType::Bishop>() & bb) {
            os << 'b';
        } else if (pos.pieces_cp<Color::BLACK, PieceType::Rook>() & bb) {
            os << 'r';
        } else if (pos.pieces_cp<Color::BLACK, PieceType::Queen>() & bb) {
            os << 'q';
        } else if (pos.pieces_cp<Color::BLACK, PieceType::King>() & bb) {
            os << 'k';
        } else {
            os << '.';
        }
        os << ' ';

        if (i % 8 == 7)
        {
            if (i/8 != 0)
                os << "\n  " << i/8 << ' ';
            i -= 16;
        }

        i++;
    }
    os << "\n    a b c d e f g h\n\n";

    os << "Castling : ";
    os << (pos.white_can_castle_k() ? "K" : "");
    os << (pos.white_can_castle_q() ? "Q" : "");
    os << (pos.black_can_castle_k() ? "k" : "");
    os << (pos.black_can_castle_q() ? "q" : "");
    os << '\n';
    if (pos.ep() == NO_SQUARE) {
        os << "EP       : -\n";
    } else {
        os << "EP       : " << pos.ep() << '\n';
    }
    os <<     "Turn     : " << (pos.turn() == Color::WHITE ? 'w' : 'b');

    return os;

}


#endif
