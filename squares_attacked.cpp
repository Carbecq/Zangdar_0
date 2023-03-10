#include "movegen.h"
#include "Board.h"

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
        mask |= movegen::knight_moves(fr);
    }

    // Bishops
    bb = pieces_cp<C, PieceType::Bishop>();
    while (bb) {
        int fr = next_square(bb);
        mask |= movegen::bishop_moves(fr, ~non_occupied());
    }

    // Rooks
    bb = pieces_cp<C, PieceType::Rook>();
    while (bb) {
        int fr = next_square(bb);
        mask |= movegen::rook_moves(fr, ~non_occupied());
    }

    // Queens
    bb = pieces_cp<C, PieceType::Queen>();
    while (bb) {
        int fr = next_square(bb);
        mask |= movegen::queen_moves(fr, ~non_occupied());
    }

    // King
    mask |= movegen::king_moves(king_position<C>());

    return mask;
}

// Explicit instantiations.
template Bitboard Board::squares_attacked<WHITE>() const noexcept ;
template Bitboard Board::squares_attacked<BLACK>() const noexcept ;
