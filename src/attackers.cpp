#include "Board.h"
#include "Attacks.h"

//! \brief Retourne le bitboard de toutes les pièces du camp "C" attaquant la case "sq"
template <Color C>
[[nodiscard]] constexpr Bitboard Board::attackers(const int sq) const noexcept
{
    // il faut regarder les attaques de pions depuis l'autre camp
    return( (Attacks::pawn_attacks<~C>(sq)         & occupancy_cp<C, PAWN>())                                           |
            (Attacks::knight_moves(sq)             & occupancy_cp<C, KNIGHT>())                                         |
            (Attacks::king_moves(sq)               & occupancy_cp<C, KING>())                                           |
            (Attacks::bishop_moves(sq, occupancy_all()) & (occupancy_cp<C, BISHOP>() | occupancy_cp<C, QUEEN>())) |
            (Attacks::rook_moves(sq,   occupancy_all()) & (occupancy_cp<C, ROOK>()   | occupancy_cp<C, QUEEN>())) );
}

//! \brief  Retourne le Bitboard de TOUS les attaquants (Blancs et Noirs) de la case "sq"
[[nodiscard]] Bitboard Board::all_attackers(const int sq, const Bitboard occ) const noexcept
{
    return( (Attacks::pawn_attacks(BLACK, sq) & occupancy_cp<WHITE, PAWN>())       |
            (Attacks::pawn_attacks(WHITE, sq) & occupancy_cp<BLACK, PAWN>())       |
            (Attacks::knight_moves(sq)        & typePiecesBB[KNIGHT])                         |
            (Attacks::king_moves(sq)          & typePiecesBB[KING])                           |
            (Attacks::bishop_moves(sq, occ)   & (typePiecesBB[BISHOP] | typePiecesBB[QUEEN])) |
            (Attacks::rook_moves(sq,   occ)   & (typePiecesBB[ROOK]   | typePiecesBB[QUEEN])) );
}


//! \brief Retourne le Bitboard des cases attaquées
template <Color C>
[[nodiscard]] constexpr Bitboard Board::squares_attacked() const noexcept {
    Bitboard mask = 0ULL;

    // Pawns
    if constexpr (C == Color::WHITE) {
        const auto pawns = occupancy_cp<C, PAWN>();
        mask |= BB::north_east(pawns);
        mask |= BB::north_west(pawns);
    } else {
        const auto pawns = occupancy_cp<C, PAWN>();
        mask |= BB::south_east(pawns);
        mask |= BB::south_west(pawns);
    }

    // Knights
    Bitboard bb = occupancy_cp<C, KNIGHT>();
    while (bb) {
        int fr = BB::pop_lsb(bb);
        mask |= Attacks::knight_moves(fr);
    }

    // Bishops
    bb = occupancy_cp<C, BISHOP>();
    while (bb) {
        int fr = BB::pop_lsb(bb);
        mask |= Attacks::bishop_moves(fr, ~occupancy_none());
    }

    // Rooks
    bb = occupancy_cp<C, ROOK>();
    while (bb) {
        int fr = BB::pop_lsb(bb);
        mask |= Attacks::rook_moves(fr, ~occupancy_none());
    }

    // Queens
    bb = occupancy_cp<C, QUEEN>();
    while (bb) {
        int fr = BB::pop_lsb(bb);
        mask |= Attacks::queen_moves(fr, ~occupancy_none());
    }

    // King
    mask |= Attacks::king_moves(king_square<C>());

    return mask;
}

//! \brief  Retourne le Bitboard des attaques de tous les pions de la couleur C
template <Color C>
[[nodiscard]] constexpr Bitboard Board::all_pawn_attacks(const Bitboard pawns)
{
    if constexpr (C == WHITE)
        return BB::shift<NORTH_WEST>(pawns) | BB::shift<NORTH_EAST>(pawns);
    else
        return BB::shift<SOUTH_WEST>(pawns) | BB::shift<SOUTH_EAST>(pawns);
}

template <Color C>
uint64_t Board::discoveredAttacks(int sq)
{
    uint64_t enemy    = colorPiecesBB[~C];
    uint64_t occupiedBB = occupancy_all();

    uint64_t rAttacks = Attacks::rook_moves(sq, occupiedBB);
    uint64_t bAttacks = Attacks::bishop_moves(sq, occupiedBB);

    uint64_t rooks   = (enemy & typePiecesBB[ROOK]) & ~rAttacks;
    uint64_t bishops = (enemy & typePiecesBB[BISHOP]) & ~bAttacks;

    return (  rooks &   Attacks::rook_moves(sq, occupiedBB & ~rAttacks))
           | (bishops & Attacks::bishop_moves(sq, occupiedBB & ~bAttacks));
}

// Explicit instantiations.
template Bitboard Board::squares_attacked<WHITE>() const noexcept ;
template Bitboard Board::squares_attacked<BLACK>() const noexcept ;

template Bitboard Board::attackers<WHITE>(const int sq) const noexcept;
template Bitboard Board::attackers<BLACK>(const int sq) const noexcept;

template Bitboard Board::all_pawn_attacks<WHITE>(const Bitboard pawns);
template Bitboard Board::all_pawn_attacks<BLACK>(const Bitboard pawns);

template uint64_t Board::discoveredAttacks<WHITE>(int sq);
template uint64_t Board::discoveredAttacks<BLACK>(int sq);
