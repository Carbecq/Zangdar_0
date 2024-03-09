#ifndef LIBCHESS_POSITION_HPP
#define LIBCHESS_POSITION_HPP

#include "MoveList.h"
#include "Bitboard.h"
#include "types.h"
#include "defines.h"
#include "zobrist.h"
#include <ostream>
#include <string>
#include <vector>
#include "Move.h"
#include "evaluate.h"
#include "Attacks.h"


// structure destinée à stocker l'historique de make_move.
// celle-ci sera nécessaire pour effectuer un unmake_move
struct UndoInfo
{
    U64  hash      = 0;          // nombre unique (?) correspondant à la position
    U64  pawn_hash = 0;
    MOVE move      = Move::MOVE_NONE;
    int  ep_square;              // case en-passant : si les blancs jouent e2-e4, la case est e3
    int  halfmove_counter = 0;   // nombre de coups depuis une capture, ou un movement de pion
    U32  castling;               // droit au roque
};

//=================================== evaluation

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
    [[nodiscard]] constexpr Bitboard occupancy_cp() const noexcept { return colorPiecesBB[C] & typePiecesBB[P]; }

    //! \brief  Retourne le bitboard de toutes les pièces Blanches et Noires
    [[nodiscard]] constexpr Bitboard occupancy_all() const noexcept { return colorPiecesBB[WHITE] | colorPiecesBB[BLACK]; }

    //! \brief  Retourne le bitboard de toutes les cases vides
    [[nodiscard]] constexpr Bitboard occupancy_none() const noexcept { return ~occupancy_all(); }

    //! \brief  Retourne le bitboard des Fous et des Dames
    template<Color C>
    constexpr Bitboard diagonal_sliders() const
    {
        return C == WHITE
                   ? occupancy_cp<WHITE, BISHOP>() | occupancy_cp<WHITE, QUEEN>()
                   : occupancy_cp<BLACK, BISHOP>() | occupancy_cp<BLACK, QUEEN>();
    }

    //! \brief  Retourne le bitboard des Tours et des Dames
    template<Color C>
    constexpr Bitboard orthogonal_sliders() const
    {
        return C == WHITE
                   ? occupancy_cp<WHITE, ROOK>() | occupancy_cp<WHITE, QUEEN>()
                   : occupancy_cp<BLACK, ROOK>() | occupancy_cp<BLACK, QUEEN>();
    }

    //! \brief Retourne le bitboard de toutes les pièces du camp "C" attaquant la case "sq"
    template <Color C>
    [[nodiscard]] constexpr Bitboard attackers(const int sq) const noexcept;

    //! \brief  Retourne le Bitboard de TOUS les attaquants (Blancs et Noirs) de la case "sq"
    [[nodiscard]] Bitboard all_attackers(const int sq, const Bitboard occ) const noexcept;

    template <Color C>
    [[nodiscard]] constexpr Bitboard all_pawn_attacks(const Bitboard pawns);


    //! \brief Returns an attack bitboard where sliders are allowed
    //! to xray other sliders moving the same directions
    //  code venant de Weiss
    template<Color C>
    [[nodiscard]] Bitboard XRayBishopAttack(const int sq)
    {
        Bitboard occ = occupancy_all() ^ occupancy_cp<C, QUEEN>() ^ occupancy_cp<C, BISHOP>();
        return(Attacks::bishop_moves(sq, occ));
    }
    template<Color C>
    [[nodiscard]] Bitboard XRayRookAttack(const int sq)
    {
        Bitboard occ = occupancy_all() ^ occupancy_cp<C, QUEEN>() ^ occupancy_cp<C, ROOK>();
        return(Attacks::rook_moves(sq, occ));
    }
    template<Color C>
    [[nodiscard]] Bitboard XRayQueenAttack(const int sq)
    {
        Bitboard occ = occupancy_all() ^ occupancy_cp<C, QUEEN>() ^ occupancy_cp<C, ROOK>() ^ occupancy_cp<C, BISHOP>();
        return(Attacks::queen_moves(sq, occ));
    }


    //        switch (pt)
    //        {
    //        case BISHOP:
    //            occ ^= pieces_cp<C, Queen>() ^ pieces_cp<C, BISHOP>();
    //            return(Attacks::bishop_moves(sq, occ));
    //            break;
    //        case ROOK:
    //            occ ^= pieces_cp<C, Queen>() ^ pieces_cp<C, ROOK>();
    //            return(Attacks::rook_moves(sq, occ));
    //            break;
    //        case Queen:
    //            occ ^= pieces_cp<C, Queen>() ^ pieces_cp<C, ROOK>() ^ pieces_cp<C, BISHOP>();
    //            return(Attacks::queen_moves(sq, occ));
    //            break;
    //        }
    //    }

    void set_fen(const std::string &fen, bool logTactics) noexcept;
    [[nodiscard]] std::string get_fen() const noexcept;
    void mirror_fen(const std::string &fen, bool logTactics);

    [[nodiscard]] constexpr int get_halfmove_counter() const noexcept { return halfmove_counter; }
    [[nodiscard]] constexpr int get_gamemove_counter() const noexcept { return gamemove_counter;     }
    [[nodiscard]] constexpr int get_fullmove_counter() const noexcept { return fullmove_counter; }

    //! \brief  Retourne la position du roi
    template<Color C>
    [[nodiscard]] constexpr int king_square() const noexcept { return x_king[C]; }

    //! \brief Retourne le bitboard des cases attaquées
    template<Color C>
    [[nodiscard]] constexpr Bitboard squares_attacked() const noexcept;

    //! \brief  Détermine si la case sq est attaquée par le camp C
    template<Color C>
    [[nodiscard]] constexpr bool square_attacked(const int sq) const noexcept { return attackers<C>(sq) > 0; }

    //! \brief  Retourne le bitboard des pièces attaquant le roi
    template<Color C>
    [[nodiscard]] constexpr Bitboard checkers() const noexcept { return attackers<C>(king_square<C>()); }

    //! \brief  Détermine si le roi est en échec
    template<Color C>
    [[nodiscard]] constexpr bool is_in_check() const noexcept { return square_attacked<~C>(king_square<C>()); }

    template<Color C> constexpr void legal_moves(MoveList &ml) noexcept;
    template<Color C> constexpr void legal_noisy(MoveList &ml) noexcept;
    template<Color C> constexpr void legal_quiet(MoveList &ml) noexcept;
    template<Color C> constexpr void legal_evasions(MoveList &ml) noexcept;

    template<Color C> void apply_token(const std::string &token) noexcept;

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

    template<Color C> constexpr void make_move(const MOVE move) noexcept;
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
        bb = occupancy_cp<WHITE, PAWN>();
        while (bb) {
            int sq = BB::pop_lsb(bb);
            khash ^= piece_key[WHITE][PAWN][sq];
            phash ^= piece_key[WHITE][PAWN][sq];
        }
        bb = occupancy_cp<WHITE, KNIGHT>();
        while (bb) {
            int sq = BB::pop_lsb(bb);
            khash ^= piece_key[WHITE][KNIGHT][sq];
        }
        bb = occupancy_cp<WHITE, BISHOP>();
        while (bb) {
            int sq = BB::pop_lsb(bb);
            khash ^= piece_key[WHITE][BISHOP][sq];
        }
        bb = occupancy_cp<WHITE, ROOK>();
        while (bb) {
            int sq = BB::pop_lsb(bb);
            khash ^= piece_key[WHITE][ROOK][sq];
        }
        bb = occupancy_cp<WHITE, QUEEN>();
        while (bb) {
            int sq = BB::pop_lsb(bb);
            khash ^= piece_key[WHITE][QUEEN][sq];
        }
        bb = occupancy_cp<WHITE, KING>();
        while (bb) {
            int sq = BB::pop_lsb(bb);
            khash ^= piece_key[WHITE][KING][sq];
        }
        bb = occupancy_cp<BLACK, PAWN>();
        while (bb) {
            int sq = BB::pop_lsb(bb);
            khash ^= piece_key[BLACK][PAWN][sq];
            phash ^= piece_key[BLACK][PAWN][sq];
        }
        bb = occupancy_cp<BLACK, KNIGHT>();
        while (bb) {
            int sq = BB::pop_lsb(bb);
            khash ^= piece_key[BLACK][KNIGHT][sq];
        }
        bb = occupancy_cp<BLACK, BISHOP>();
        while (bb) {
            int sq = BB::pop_lsb(bb);
            khash ^= piece_key[BLACK][BISHOP][sq];
        }
        bb = occupancy_cp<BLACK, ROOK>();
        while (bb) {
            int sq = BB::pop_lsb(bb);
            khash ^= piece_key[BLACK][ROOK][sq];
        }
        bb = occupancy_cp<BLACK, QUEEN>();
        while (bb) {
            int sq = BB::pop_lsb(bb);
            khash ^= piece_key[BLACK][QUEEN][sq];
        }
        bb = occupancy_cp<BLACK, KING>();
        while (bb) {
            int sq = BB::pop_lsb(bb);
            khash ^= piece_key[BLACK][KING][sq];
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
        return( (colorPiecesBB[WHITE] & BB::sq2BB(sq)) ? WHITE : BLACK);
    }

    //! \brief  Retourne le type de la pièce située sur la case sq
    [[nodiscard]] constexpr PieceType piece_on(const int sq) const noexcept
    {
        for (int i = PAWN; i <= KING; ++i) {
            if (typePiecesBB[i] & BB::sq2BB(sq)) {
                return PieceType(i);
            }
        }
        return NO_TYPE;
    }

    [[nodiscard]] constexpr int ep() const noexcept { return ep_square; }
    [[nodiscard]] constexpr std::uint64_t get_hash() const noexcept { return hash; }
    [[nodiscard]] constexpr std::uint64_t get_pawn_hash() const noexcept { return pawn_hash; }

    bool valid() const noexcept;
    [[nodiscard]] std::string display() const noexcept;

    template<Color C> Bitboard getNonPawnMaterial() const noexcept{

        return (occupancy_cp<C, KNIGHT>() |
                occupancy_cp<C, BISHOP>() |
                occupancy_cp<C, ROOK>()   |
                occupancy_cp<C, QUEEN>() );
    }


    template<Color C> constexpr int non_pawn_count() const
    {
        return (BB::count_bit(colorPiecesBB[C]
                       ^ occupancy_cp<C, PAWN>()
                       ^ occupancy_cp<C, KING>() ) );
    }

    Score evaluate();
    Score evaluate_pieces(EvalInfo& ei);

    template <Color C> Score evaluate_pawns(EvalInfo& ei);
    template <Color C> Score evaluate_knights(EvalInfo& ei);
    template <Color C> Score evaluate_bishops(EvalInfo& ei);
    template <Color C> Score evaluate_rooks(EvalInfo& ei);
    template <Color C> Score evaluate_queens(EvalInfo& ei);
    template <Color C> Score evaluate_king(EvalInfo& ei);
    template <Color C> Score evaluate_threats(const EvalInfo& ei);
    template <Color C> Score evaluate_passed(EvalInfo& ei);
    template <Color C> Score evaluate_space(EvalInfo& ei);
    template <Color C> Score evaluate_kingspawns(EvalInfo& ei);

    Score evaluate_closedness(EvalInfo& ei);
    Score evaluate_complexity(EvalInfo& ei, Score eval);

    int   scale_factor(const Score eval);
    void  init_eval_info(EvalInfo& ei);
    Score probe_pawn_cache(EvalInfo& ei);

    bool material_draw(void);

    bool fast_see(const MOVE move, const int threshold) const;
    void test_value(const std::string& fen );


    //====================================================================
    //! \brief  Détermine s'il y a eu 50 coups sans prise ni coup de pion
    //--------------------------------------------------------------------
    [[nodiscard]] constexpr bool fiftymoves() const noexcept { return halfmove_counter >= 100; }

    //====================================================================
    //! \brief  Détermine s'il y a eu répétition de la même position
    //! Pour cela, on compare le hash code de la position.
    //! Voir Ethereal
    //--------------------------------------------------------------------
    [[nodiscard]] bool is_repetition(int ply) const noexcept
    {
        int reps = 0;

        // Look through hash histories for our moves
        for (int i = gamemove_counter - 2; i >= 0; i -= 2) {

            // No draw can occur before a zeroing move
            if (i < gamemove_counter - halfmove_counter)
                break;

            // Check for matching hash with a two fold after the root,
            // or a three fold which occurs in part before the root move
            if (    game_history[i].hash == hash
                && (i > gamemove_counter - ply || ++reps == 2))
                return true;
        }

        return false;
    }

    //=============================================================================
    //! \brief  Détermine si la position est nulle
    //-----------------------------------------------------------------------------
    [[nodiscard]] bool is_draw(int ply) const noexcept
    {
        return ((is_repetition(ply) || fiftymoves()));
    }

    //=============================================================================
    //! \brief  Met une pièce à la case indiquée
    //-----------------------------------------------------------------------------
    void set_piece(const int sq, const Color s, const PieceType p) noexcept
    {
        colorPiecesBB[s] |= BB::sq2BB(sq);
        typePiecesBB[p]  |= BB::sq2BB(sq);
        pieceOn[sq] = p;
    }

    bool test_mirror(const std::string &line);

    //! \brief  Calcule la phase de la position sur 24 points.
    //! Cette phase dépend des pièces sur l'échiquier
    //! La phase va de 0 (EndGame) à 24 (MiddleGame), dans le cas où aucun pion n'a été promu.
    int  get_phase24();

    //! \brief Calcule la phase de la position sur 256 points.
    //! ceci pour avoir une meilleure granulométrie ?
    //! ouverture     : phase24 = 24 : phase256 = 256,5
    //! fin de partie :         =  0 :          = 0,5
    int get_phase256(int phase24) { return (phase24 * 256 + 12) / 24; }

    //------------------------------------------------------------Syzygy
    void TBScore(const unsigned wdl, const unsigned dtz, int &score, int &bound) const;
    bool probe_wdl(int &score, int &bound, int ply) const;
    MOVE convertPyrrhicMove(unsigned result) const;
    bool probe_root(MOVE& move) const;

    //------------------------------------------------------------attackers
    template <Color C> Bitboard discoveredAttacks(int sq);

    //*************************************************************************
    //*************************************************************************
    //*************************************************************************

    //------------------------------------------------------- la position
    Bitboard colorPiecesBB[2] = {0ULL};     // bitboard des pièces pour chaque couleur
    Bitboard typePiecesBB[7]  = {0ULL};     // bitboard des pièces pour chaque type de pièce
    std::array<PieceType, 64> pieceOn;      // donne le type de la pièce occupant la case indiquée
    int x_king[2];                          // position des rois

    Color side_to_move = Color::WHITE; // camp au trait
    int   ep_square    = NO_SQUARE;  // case en-passant : si les blancs jouent e2-e4, la case est e3
    U32   castling     = CASTLE_NONE; // droit au roque

    /*
     * The Halfmove Clock inside an chess position object takes care of enforcing the fifty-move rule.
     * This counter is reset after captures or pawn moves, and incremented otherwise.
     * Also moves which lose the castling rights, that is rook- and king moves from their initial squares,
     * including castling itself, increment the Halfmove Clock.
     * However, those moves are irreversible in the sense to reverse the same rights -
     * since once a castling right is lost, it is lost forever, as considered in detecting repetitions.
     *
     */

    int halfmove_counter = 0; // nombre de demi-coups depuis la dernière capture ou le dernier mouvement de pion.
    int fullmove_counter = 1; // le nombre de coups complets. Il commence à 1 et est incrémenté de 1 après le coup des noirs.
    int gamemove_counter = 0; // nombre de demi-coups de la partie

    U64 hash           = 0ULL;  // nombre unique (?) correspondant à la position (clef Zobrist)
    U64 pawn_hash      = 0ULL;  // hash uniquement pour les pions

    std::vector<std::string> best_moves;  // meilleur coup (pour les test tactique)
    std::vector<std::string> avoid_moves; // coup à éviter (pour les test tactique)

    std::array<UndoInfo, MAX_HIST> game_history;



};  // class Board


//================================================================================
//! \brief  Affichage de l'échiquier
//--------------------------------------------------------------------------------
inline std::ostream &operator << (std::ostream &os, const Board &pos) noexcept
{
    int i = 56;
    os << std::endl;

    os << "  8 ";

    while (i >= 0) {
        const auto sq = i;
        const auto bb = BB::sq2BB(sq);

        if (pos.occupancy_cp<Color::WHITE, PAWN>() & bb) {
            os << 'P';
        } else if (pos.occupancy_cp<Color::WHITE, KNIGHT>() & bb) {
            os << 'N';
        } else if (pos.occupancy_cp<Color::WHITE, BISHOP>() & bb) {
            os << 'B';
        } else if (pos.occupancy_cp<Color::WHITE, ROOK>() & bb) {
            os << 'R';
        } else if (pos.occupancy_cp<Color::WHITE, QUEEN>() & bb) {
            os << 'Q';
        } else if (pos.occupancy_cp<Color::WHITE, KING>() & bb) {
            os << 'K';
        } else if (pos.occupancy_cp<Color::BLACK, PAWN>() & bb) {
            os << 'p';
        } else if (pos.occupancy_cp<Color::BLACK, KNIGHT>() & bb) {
            os << 'n';
        } else if (pos.occupancy_cp<Color::BLACK, BISHOP>() & bb) {
            os << 'b';
        } else if (pos.occupancy_cp<Color::BLACK, ROOK>() & bb) {
            os << 'r';
        } else if (pos.occupancy_cp<Color::BLACK, QUEEN>() & bb) {
            os << 'q';
        } else if (pos.occupancy_cp<Color::BLACK, KING>() & bb) {
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
