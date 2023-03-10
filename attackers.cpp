#include "Board.h"
#include "movegen.h"

//! \brief retourne le bitboard de toutes les pièces du camp "C" attaquant la case "sq"
//!
template <Color C>
[[nodiscard]] constexpr Bitboard Board::attackers(const int sq) const noexcept
{
    // il faut regarder les attaques de pions depuis l'autre camp
    Bitboard mask = movegen::pawn_attacks(!C, sq) & pieces_cp<C, PieceType::Pawn>();

    mask |= movegen::knight_moves(sq) & pieces_cp<C, PieceType::Knight>();
    mask |= movegen::bishop_moves(sq, ~non_occupied()) & (pieces_cp<C, PieceType::Bishop>() | pieces_cp<C, PieceType::Queen>());
    mask |= movegen::rook_moves(sq,   ~non_occupied()) & (pieces_cp<C, PieceType::Rook>()   | pieces_cp<C, PieceType::Queen>());
    mask |= movegen::king_moves(sq) & pieces_cp<C, PieceType::King>();

    return mask;
}

// Explicit instantiations.

template Bitboard Board::attackers<WHITE>(const int sq) const noexcept;
template Bitboard Board::attackers<BLACK>(const int sq) const noexcept;
