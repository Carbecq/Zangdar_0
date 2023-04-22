#include "Board.h"
#include "Square.h"

//=============================================================
//! \brief  Enlève un coup
//-------------------------------------------------------------
template <Color C> constexpr void Board::undo_move() noexcept
{
    // Swap sides
    side_to_move = ~side_to_move;

    game_clock--;

    const auto &move    = my_history[game_clock].move;
    const auto them     = !C;
    const auto to       = Move::dest(move);         //.to();
    const auto from     = Move::from(move);         //.from();
    const auto piece    = Move::piece(move);
    const auto captured = Move::captured(move);
    const auto promo    = Move::promotion(move);

    // En passant
    // back : Returns a reference to the last element in the vector.
    ep_square = my_history[game_clock].ep_square;

    // Halfmoves
    halfmove_clock = my_history[game_clock].halfmove_clock;

    // Fullmoves
    fullmove_clock -= (C == Color::BLACK);

    // Déplacement du roi
    if (piece == PieceType::King)
        x_king[C] = from;

    // Castling
    castling = my_history[game_clock].castling;

#ifdef HASH
    hash = my_history[game_clock].hash;
#endif

    // Remove piece
    flip(colorPiecesBB[C],    to);
    flip(typePiecesBB[piece],  to);

    // Add piece
    flip(colorPiecesBB[C], from);
    flip(typePiecesBB[piece], from);

    switch (Move::type(move))
    {
    case MoveType::Normal:
        cpiece[from]= piece;
        cpiece[to]  = PieceType::NO_TYPE;
        break;
    case MoveType::Double:
        cpiece[from]= piece;
        cpiece[to]  = PieceType::NO_TYPE;
        break;
    case MoveType::Capture:
        cpiece[from]= piece;
        flip(colorPiecesBB[them], to);
        flip(typePiecesBB[captured], to);
        cpiece[to]  = captured;
        break;
    case MoveType::EnPassant:
        // Replace the captured pawn
        if (C == Color::WHITE)
        {
            flip(typePiecesBB[PieceType::Pawn], Square::south(to));
            flip(colorPiecesBB[Color::BLACK], Square::south(to));

            cpiece[from]= piece;
            cpiece[to]  = PieceType::NO_TYPE;
            cpiece[Square::south(to)] = PieceType::Pawn;
        }
        else
        {
            flip(typePiecesBB[PieceType::Pawn], Square::north(to));
            flip(colorPiecesBB[Color::WHITE], Square::north(to));

            cpiece[from]= piece;
            cpiece[to]  = PieceType::NO_TYPE;
            cpiece[Square::north(to)] = PieceType::Pawn;
        }
        break;

        //==================================================================================
    case MoveType::KingCastle:
        // Move the rook
        flip2(colorPiecesBB[C],  ksc_castle_rook_from[C], ksc_castle_rook_to[C]);
        flip2(typePiecesBB[PieceType::Rook], ksc_castle_rook_from[C], ksc_castle_rook_to[C]);

        cpiece[from]= piece;
        cpiece[to]  = PieceType::NO_TYPE;
        cpiece[ksc_castle_rook_from[C]] = PieceType::Rook;
        cpiece[ksc_castle_rook_to[C]]   = PieceType::NO_TYPE;
        break;

        //==================================================================================
    case MoveType::QueenCastle:
        // Move the rook
        flip2(colorPiecesBB[C], qsc_castle_rook_from[C], qsc_castle_rook_to[C]);
        flip2(typePiecesBB[PieceType::Rook], qsc_castle_rook_from[C], qsc_castle_rook_to[C]);

        cpiece[from]= piece;
        cpiece[to]  = PieceType::NO_TYPE;
        cpiece[qsc_castle_rook_from[C]] = PieceType::Rook;
        cpiece[qsc_castle_rook_to[C]]   = PieceType::NO_TYPE;
        break;

        //==================================================================================
    case MoveType::Promotion:
        // Replace piece with pawn
        flip(typePiecesBB[PieceType::Pawn], to);
        flip(typePiecesBB[promo], to);

        cpiece[from]= PieceType::Pawn;
        cpiece[to]  = PieceType::NO_TYPE;
        break;

        //==================================================================================
    case MoveType::PromotionCapture:
        // Replace pawn with piece
        flip(typePiecesBB[PieceType::Pawn], to);
        flip(typePiecesBB[promo], to);

        // Replace the captured piece
        flip(typePiecesBB[captured], to);
        flip(colorPiecesBB[them], to);

        cpiece[from]= PieceType::Pawn;
        cpiece[to]  = captured;
        break;

        //==================================================================================
    default:
        break;
    }

    assert(valid<C>());
}

//===================================================================
//! \brief  Enlève un NullMove
//-------------------------------------------------------------------
template <Color C> constexpr void Board::undo_nullmove() noexcept
{
    // Swap sides
    side_to_move = ~side_to_move;

    game_clock--;

    // En passant
    ep_square = my_history[game_clock].ep_square;

    // Halfmoves
    halfmove_clock = my_history[game_clock].halfmove_clock;

    // Fullmoves
    fullmove_clock -= (C == Color::BLACK);

    // Castling
    castling = my_history[game_clock].castling;

#ifdef HASH
    hash = my_history[game_clock].hash;
#endif

    assert(valid<C>());
}



// Explicit instantiations.

template void Board::undo_move<WHITE>() noexcept ;
template void Board::undo_move<BLACK>() noexcept ;

template void Board::undo_nullmove<WHITE>() noexcept ;
template void Board::undo_nullmove<BLACK>() noexcept ;
