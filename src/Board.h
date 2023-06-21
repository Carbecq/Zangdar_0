#ifndef LIBCHESS_POSITION_HPP
#define LIBCHESS_POSITION_HPP

#include "MoveList.h"
#include "bitboard.h"
#include "types.h"
#include "defines.h"
#include "Move.h"
#include "zobrist.h"
#include <cstring>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

struct Mask
{
    int direction[64];
};

// structure destinée à stocker l'historique de make_move.
// celle-ci sera nécessaire pour effectuer un unmake_move
struct UndoInfo
{
    U64 hash      = 0;      // nombre unique (?) correspondant à la position
    U64 pawn_hash = 0;
    U32 move;
    int ep_square;          // case en-passant : si les blancs jouent e2-e4, la case est e3
    int halfmove_clock = 0; // nombre de coups depuis une capture, ou un movement de pion
    U32 castling;           // droit au roque
};

/*******************************************************
 ** Droit au roque
 **---------------------------------------------------*/
enum Castle { CASTLE_NONE = 0, CASTLE_WK = 1, CASTLE_WQ = 2, CASTLE_BK = 4, CASTLE_BQ = 8 };

constexpr const int ksc_castle_rook_from[] = {H1, H8};
constexpr const int ksc_castle_rook_to[] = {F1, F8};

constexpr const int qsc_castle_rook_from[] = {A1, A8};
constexpr const int qsc_castle_rook_to[] = {D1, D8};

constexpr const int castle_king_from[] = {E1, E8};
constexpr const int ksc_castle_king_to[] = {G1, G8};
constexpr const int qsc_castle_king_to[] = {C1, C8};


class Board
{
public:
    [[nodiscard]] Board() noexcept;
    [[nodiscard]] Board(const std::string &fen) noexcept;

    void clear() noexcept;
    void parse_position(std::istringstream &is);

    //! \brief  Retourne le camp à jouer
    [[nodiscard]] constexpr Color turn() const noexcept { return side_to_move; }

    //! \brief  Retourne le bitboard des pièces de la couleur indiquée
    template<Color C>
    [[nodiscard]] constexpr Bitboard occupancy_c() const noexcept { return colorPiecesBB[C]; }

    //! \brief  Retourne le bitboard des pièces du type indiqué
    template<PieceType P>
    [[nodiscard]] constexpr uint64_t occupancy_p() const noexcept { return typePiecesBB[P]; }

    //! \brief  Retourne le bitboard des pièces de la couleur indiquée
    //! et du type indiqué
    template<Color C, PieceType P>
    [[nodiscard]] constexpr Bitboard pieces_cp() const noexcept { return colorPiecesBB[C] & typePiecesBB[P]; }

    //! \brief  Retourne le bitboard de toutes les pièces Blanches et Noires
    [[nodiscard]] constexpr Bitboard occupied() const noexcept { return colorPiecesBB[WHITE] | colorPiecesBB[BLACK]; }

    //! \brief  Retourne le bitboard de toutes les cases vides
    [[nodiscard]] constexpr Bitboard non_occupied() const noexcept { return ~occupied(); }

    //! \brief  Retourne le bitboard des Fous et des Dames
    template<Color C>
    constexpr Bitboard diagonal_sliders() const
    {
        return C == WHITE
                   ? pieces_cp<WHITE, PieceType::Bishop>() | pieces_cp<WHITE, PieceType::Queen>()
                   : pieces_cp<BLACK, PieceType::Bishop>() | pieces_cp<BLACK, PieceType::Queen>();
    }

    //! \brief  Retourne le bitboard des Tours et des Dames
    template<Color C>
    constexpr Bitboard orthogonal_sliders() const
    {
        return C == WHITE
                   ? pieces_cp<WHITE, PieceType::Rook>() | pieces_cp<WHITE, PieceType::Queen>()
                   : pieces_cp<BLACK, PieceType::Rook>() | pieces_cp<BLACK, PieceType::Queen>();
    }

    //! \brief Retourne le bitboard de toutes les pièces du camp "C" attaquant la case "sq"
    template <Color C>
    [[nodiscard]] constexpr Bitboard attackers(const int sq) const noexcept;

    //! \brief  Retourne le Bitboard de TOUS les attaquants (Blancs et Noirs) de la case "sq"
    [[nodiscard]] Bitboard all_attackers(const int sq, const Bitboard occ) const noexcept;

    template <Color C>
    [[nodiscard]] constexpr Bitboard all_pawn_attacks(const Bitboard pawns);

    void set_fen(const std::string &fen, bool logTactics) noexcept;
    [[nodiscard]] std::string get_fen() const noexcept;
    void mirror_fen(const std::string &fen, bool logTactics);

    [[nodiscard]] constexpr int get_halfmove_clock()         const noexcept { return halfmove_clock; }
    [[nodiscard]] constexpr int get_game_clock()             const noexcept { return game_clock;     }
    [[nodiscard]] constexpr std::size_t get_fullmove_clock() const noexcept { return fullmove_clock; }

    //! \brief  Retourne la position du roi
    template<Color C>
    [[nodiscard]] constexpr int king_position() const noexcept { return x_king[C]; }

    //! \brief Retourne le bitboard des cases attaquées
    template<Color C>
    [[nodiscard]] constexpr Bitboard squares_attacked() const noexcept;

    //! \brief  Détermine si la case sq est attaquée par le camp C
    template<Color C>
    [[nodiscard]] constexpr bool square_attacked(const int sq) const noexcept { return attackers<C>(sq) > 0; }

    //! \brief  Retourne le bitboard des pièces attaquant le roi
    template<Color C>
    [[nodiscard]] constexpr Bitboard checkers() const noexcept { return attackers<C>(king_position<C>()); }

    //! \brief  Détermine si le roi est en échec
    template<Color C>
    [[nodiscard]] constexpr bool is_in_check() const noexcept { return square_attacked<~C>(king_position<C>()); }

    template<Color C>
    constexpr void legal_moves(MoveList &ml) noexcept;
    template<Color C>
    constexpr void legal_captures(MoveList &ml) noexcept;
    template<Color C>
    constexpr void legal_evasions(MoveList &ml) noexcept;

    template<Color C>
    void apply_token(const std::string &token) noexcept;

    void verify_MvvLva();

    void add_quiet_move(MoveList &ml, int from, int dest, PieceType piece, U32 flags) const noexcept;
    void add_capture_move(
                          MoveList &ml,
                          int from,
                          int dest,
                          PieceType piece,
                          PieceType captured,
                          U32 flags) const noexcept;
    void add_quiet_promotion(MoveList &ml, int from, int dest, PieceType promo) const noexcept;
    void add_capture_promotion(MoveList &ml,
                               int from,
                               int dest,
                               PieceType captured,
                               PieceType promo) const noexcept;

    void push_quiet_moves(MoveList &ml, Bitboard attack, const int from);
    void push_capture_moves(MoveList &ml, Bitboard attack, const int from);
    void push_piece_quiet_moves(MoveList &ml, Bitboard attack, const int from, PieceType piece);
    void push_piece_capture_moves(MoveList &ml, Bitboard attack, const int from, PieceType piece);
    void push_quiet_promotions(MoveList &ml, Bitboard attack, const int dir);
    void push_capture_promotions(MoveList &ml, Bitboard attack, const int dir);
    void push_quiet_promotion(MoveList &ml, const int from, const int to);
    void push_capture_promotion(MoveList &ml, const int from, const int to);
    void push_pawn_quiet_moves(MoveList &ml, Bitboard attack, const int dir, U32 flags);
    void push_pawn_capture_moves(MoveList &ml, Bitboard attack, const int dir);

    template<Color C> [[nodiscard]] std::uint64_t perft(const int depth) noexcept;
    template<Color C> [[nodiscard]] std::uint64_t divide(const int depth) noexcept;

    template<Color C> [[nodiscard]] constexpr bool can_castle() const noexcept
    {
        switch (C) {
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

    template<Color C> [[nodiscard]] constexpr bool can_castle_k() const noexcept
    {
        switch (C) {
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

    template<Color C> [[nodiscard]] constexpr bool can_castle_q() const noexcept
    {
        switch (C) {
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

    [[nodiscard]] constexpr bool white_can_castle_k() const noexcept
    {
        return castling & CASTLE_WK;
    }
    [[nodiscard]] constexpr bool white_can_castle_q() const noexcept
    {
        return castling & CASTLE_WQ;
    }
    [[nodiscard]] constexpr bool black_can_castle_k() const noexcept
    {
        return castling & CASTLE_BK;
    }
    [[nodiscard]] constexpr bool black_can_castle_q() const noexcept
    {
        return castling & CASTLE_BQ;
    }

    template<Color C> constexpr void make_move(const U32 move) noexcept;
    template<Color C> constexpr void undo_move() noexcept;
    template<Color C> constexpr void make_nullmove() noexcept;
    template<Color C> constexpr void undo_nullmove() noexcept;

    constexpr void calculate_hash(U64& khash, U64& phash) const noexcept
    {
        khash = 0ULL;
        phash = 0ULL;
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
            phash ^= piece_key[WHITE][PieceType::Pawn][sq];
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
            phash ^= piece_key[BLACK][PieceType::Pawn][sq];
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
    }

    //! \brief  Retourne la couleur de la pièce située sur la case sq
    //! SUPPOSE qu'il y a une pièce sur cette case !!
    [[nodiscard]] constexpr Color color_on(const int sq) const noexcept
    {
        return( (colorPiecesBB[WHITE] & square_to_bit(sq)) ? WHITE : BLACK);
    }

    //! \brief  Retourne le type de la pièce située sur la case sq
    [[nodiscard]] constexpr PieceType piece_on(const int sq) const noexcept
    {
        for (int i = Pawn; i <= King; ++i) {
            if (typePiecesBB[i] & square_to_bit(sq)) {
                return PieceType(i);
            }
        }
        return PieceType::NO_TYPE;
    }

    [[nodiscard]] constexpr int ep() const noexcept { return ep_square; }
    [[nodiscard]] constexpr std::uint64_t get_hash() const noexcept { return hash; }
    [[nodiscard]] constexpr std::uint64_t get_pawn_hash() const noexcept { return pawn_hash; }

    template<Color C>
    [[nodiscard]] constexpr bool valid() const noexcept;
    [[nodiscard]] std::string display() const noexcept;

    template<Color C>
    Bitboard getNonPawnMaterial() const noexcept{
        return (pieces_cp<C, Knight>() |
                pieces_cp<C, Bishop>() |
                pieces_cp<C, Rook>()   |
                pieces_cp<C, Queen>() );
    }

    template<Color C>
    constexpr bool major_pieces() const
    {
        return (Bcount(colorPiecesBB[C] ^ pieces_cp<C, PieceType::Pawn>()
                       ^ pieces_cp<C, PieceType::King>())
                > 0);
    }

    //=================================== evaluation
    template<bool Mode> [[nodiscard]] int evaluate();
    template<Color C> constexpr void fast_evaluate(Score& score, int &phase);

    Score slow_evaluate(int& phase);

    template<Color Us> Score evaluate_pawns(const Bitboard UsPawnsBB, const Bitboard OtherPawnsBB);
    template<Color Us> Score evaluate_knights(const Bitboard mobilityArea, int &phase);
    template<Color Us> Score evaluate_bishops(const Bitboard occupiedBB, const Bitboard mobilityArea, int &phase);
    template<Color Us> Score evaluate_rooks(const Bitboard UsPawnsBB, const Bitboard OtherPawnsBB, const Bitboard occupiedBB, const Bitboard mobilityArea, int &phase);
    template<Color Us> Score evaluate_queens(const Bitboard UsPawnsBB, const Bitboard occupiedBB, const Bitboard mobilityArea, int &phase);
    template<Color Us> Score evaluate_king();

    bool fast_see(const MOVE move, const int threshold) const;
    void test_value(const std::string& fen );


    void init_bitmasks();

    Bitboard colorPiecesBB[2] = {0ULL}; // occupancy board pour chaque couleur
    Bitboard typePiecesBB[7]  = {0ULL};  // bitboard pour chaque type de piece
    std::array<PieceType, 64> cpiece;   // tableau des pièces par case
    int x_king[2];                      // position des rois

    std::array<Mask, 64> allmask;

    //------------------------------------------------------- la position
    Color side_to_move = Color::WHITE; // camp au trait
    int   ep_square    = NO_SQUARE;  // case en-passant : si les blancs jouent e2-e4, la case est e3
    U32   castling     = CASTLE_NONE; // droit au roque

    int halfmove_clock = 0; // nombre de demi-coups depuis la dernière capture ou le dernier mouvement de pion.
    int game_clock     = 0; // nombre de demi-coups de la partie
    int fullmove_clock = 1; // le nombre de coups complets. Il commence à 1 et est incrémenté de 1 après le coup des noirs.

    U64 hash           = 0ULL;  // nombre unique (?) correspondant à la position (clef Zobrist)
    U64 pawn_hash      = 0ULL;  // hash uniquement pour les pions

    std::vector<std::string> best_moves;  // meilleur coup (pour les test tactique)
    std::vector<std::string> avoid_moves; // coup à éviter (pour les test tactique)

    // private:

    std::array<UndoInfo, MAX_HIST> my_history;


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

            if (my_history[i].hash == hash)
                return true;
        }
        return false;
    }

    //=============================================================================
    //! \brief  Détermine si la position est nulle
    //-----------------------------------------------------------------------------
    template<Color C>
    [[nodiscard]] constexpr bool is_draw() const noexcept
    {
        return ((is_repetition() || fiftymoves()));
    }

    //=============================================================================
    //! \brief  Met une pièce à la case indiquée
    //-----------------------------------------------------------------------------
    void set_piece(const int sq, const Color s, const PieceType p) noexcept
    {
        colorPiecesBB[s] |= square_to_bit(sq);
        typePiecesBB[p] |= square_to_bit(sq);
        cpiece[sq] = p;
    }



    /*
     * The Halfmove Clock inside an chess position object takes care of enforcing the fifty-move rule.
     * This counter is reset after captures or pawn moves, and incremented otherwise.
     * Also moves which lose the castling rights, that is rook- and king moves from their initial squares,
     * including castling itself, increment the Halfmove Clock.
     * However, those moves are irreversible in the sense to reverse the same rights -
     * since once a castling right is lost, it is lost forever, as considered in detecting repetitions.
     *
     */

    bool test_mirror(const std::string &line);

    //------------------------------------------------------------Syzygy
    void TBScore(const unsigned wdl, const unsigned dtz, int &score, int &bound) const;
    bool probe_wdl(int &score, int &bound, int ply) const;
    MOVE convertPyrrhicMove(unsigned result) const;
    bool probe_root(MOVE& move) const;

};

//================================================================================
//! \brief  Affichage de l'échiquier
//--------------------------------------------------------------------------------
inline std::ostream &operator<<(std::ostream &os, const Board &pos) noexcept
{
    int i = 56;
    os << std::endl;

    os << "  8 ";

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

        if (i % 8 == 7) {
            if (i / 8 != 0)
                os << "\n  " << i / 8 << ' ';
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
    os << "Turn     : " << (pos.turn() == Color::WHITE ? 'w' : 'b');

    return os;
}

#endif
