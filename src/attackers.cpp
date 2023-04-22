#include "Board.h"
#include "MoveGen.h"

//! \brief Retourne le bitboard de toutes les pièces du camp "C" attaquant la case "sq"
template <Color C>
[[nodiscard]] constexpr Bitboard Board::attackers(const int sq) const noexcept
{
    // il faut regarder les attaques de pions depuis l'autre camp
    return( (MoveGen::pawn_attacks(!C, sq) & pieces_cp<C, PieceType::Pawn>())                                                |
            (MoveGen::knight_moves(sq) & pieces_cp<C, PieceType::Knight>())                                                  |
            (MoveGen::king_moves(sq) & pieces_cp<C, PieceType::King>())                                                      |
            (MoveGen::bishop_moves(sq, occupied()) & (pieces_cp<C, PieceType::Bishop>() | pieces_cp<C, PieceType::Queen>())) |
            (MoveGen::rook_moves(sq,   occupied()) & (pieces_cp<C, PieceType::Rook>()   | pieces_cp<C, PieceType::Queen>())) );
}

//! \brief  Retourne le Bitboard de TOUS les attaquants (Blancs et Noirs) de la case "sq"
[[nodiscard]] Bitboard Board::all_attackers(const int sq, const Bitboard occ) const noexcept
{
    return( (MoveGen::pawn_attacks(BLACK, sq) & pieces_cp<WHITE, PieceType::Pawn>())        |
            (MoveGen::pawn_attacks(WHITE, sq) & pieces_cp<BLACK, PieceType::Pawn>())        |
            (MoveGen::knight_moves(sq) & typePiecesBB[Knight])                              |
            (MoveGen::king_moves(sq)   & typePiecesBB[King])                                |
            (MoveGen::bishop_moves(sq, occ) & (typePiecesBB[Bishop] | typePiecesBB[Queen])) |
            (MoveGen::rook_moves(sq,   occ) & (typePiecesBB[Rook]   | typePiecesBB[Queen])) );
}


//! \brief Retourne le bitboard des cases attaquèes
template <Color C>
[[nodiscard]] constexpr Bitboard Board::squares_attacked() const noexcept {
    Bitboard mask = ZERO;

    // Pawns
    if (C == Color::WHITE) {
        const auto pawns = pieces_cp<C, PieceType::Pawn>();
        mask |= north_east(pawns);
        mask |= north_west(pawns);
    } else {
        const auto pawns = pieces_cp<C, PieceType::Pawn>();
        mask |= south_east(pawns);
        mask |= south_west(pawns);
    }

    // Knights
    Bitboard bb = pieces_cp<C, PieceType::Knight>();
    while (bb) {
        int fr = next_square(bb);
        mask |= MoveGen::knight_moves(fr);
    }

    // Bishops
    bb = pieces_cp<C, PieceType::Bishop>();
    while (bb) {
        int fr = next_square(bb);
        mask |= MoveGen::bishop_moves(fr, ~non_occupied());
    }

    // Rooks
    bb = pieces_cp<C, PieceType::Rook>();
    while (bb) {
        int fr = next_square(bb);
        mask |= MoveGen::rook_moves(fr, ~non_occupied());
    }

    // Queens
    bb = pieces_cp<C, PieceType::Queen>();
    while (bb) {
        int fr = next_square(bb);
        mask |= MoveGen::queen_moves(fr, ~non_occupied());
    }

    // King
    mask |= MoveGen::king_moves(king_position<C>());

    return mask;
}

// Explicit instantiations.
template Bitboard Board::squares_attacked<WHITE>() const noexcept ;
template Bitboard Board::squares_attacked<BLACK>() const noexcept ;

// Explicit instantiations.

template Bitboard Board::attackers<WHITE>(const int sq) const noexcept;
template Bitboard Board::attackers<BLACK>(const int sq) const noexcept;
