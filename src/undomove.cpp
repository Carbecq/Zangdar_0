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
    const auto dest     = Move::dest(move);
    const auto from     = Move::from(move);
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
    hash      = my_history[game_clock].hash;
    pawn_hash = my_history[game_clock].pawn_hash;
#endif

    // Remove piece
    flip(colorPiecesBB[C],    dest);
    flip(typePiecesBB[piece],  dest);

    // Add piece
    flip(colorPiecesBB[C], from);
    flip(typePiecesBB[piece], from);

    //====================================================================================
    //  Coup normal (pas spécial)
    //------------------------------------------------------------------------------------
    if (Move::flags(move) == Move::FLAG_NONE)
    {

        //====================================================================================
        //  Déplacement simple
        //------------------------------------------------------------------------------------
        if (Move::is_depl(move))
        {
            cpiece[from] = piece;
            cpiece[dest] = PieceType::NO_TYPE;
        }

        //====================================================================================
        //  Captures
        //------------------------------------------------------------------------------------
        else if (Move::is_capturing(move))
        {
            //====================================================================================
            //  Promotion avec capture
            //------------------------------------------------------------------------------------
            if (Move::is_promoting(move))
            {
                // Replace pawn with piece
                flip(typePiecesBB[PieceType::Pawn], dest);
                flip(typePiecesBB[promo], dest);

                // Replace the captured piece
                flip(typePiecesBB[captured], dest);
                flip(colorPiecesBB[them], dest);

                cpiece[from]= PieceType::Pawn;
                cpiece[dest]  = captured;
            }

            //====================================================================================
            //  Capture simple
            //------------------------------------------------------------------------------------
            else
            {
                cpiece[from]= piece;
                flip(colorPiecesBB[them], dest);
                flip(typePiecesBB[captured], dest);
                cpiece[dest]  = captured;
            }
        }

        //====================================================================================
        //  Promotion simple
        //------------------------------------------------------------------------------------
        else if (Move::is_promoting(move))
        {
            // Replace piece with pawn
            flip(typePiecesBB[PieceType::Pawn], dest);
            flip(typePiecesBB[promo], dest);

            cpiece[from]= PieceType::Pawn;
            cpiece[dest]  = PieceType::NO_TYPE;
        }
    }

    //====================================================================================
    //  Coup spécial : Double, EnPassant, Roque
    //------------------------------------------------------------------------------------
    else
    {
        //====================================================================================
        //  Poussée double de pions
        //------------------------------------------------------------------------------------
        if (Move::is_double(move))
        {
            cpiece[from]= piece;
            cpiece[dest]  = PieceType::NO_TYPE;
        }

        //====================================================================================
        //  Prise en passant
        //------------------------------------------------------------------------------------
        else if (Move::is_enpassant(move))
        {
            // Replace the captured pawn
            if (C == Color::WHITE)
            {
                flip(typePiecesBB[PieceType::Pawn], Square::south(dest));
                flip(colorPiecesBB[Color::BLACK], Square::south(dest));

                cpiece[from]= PieceType::Pawn;
                cpiece[dest]  = PieceType::NO_TYPE;
                cpiece[Square::south(dest)] = PieceType::Pawn;
            }
            else
            {
                flip(typePiecesBB[PieceType::Pawn], Square::north(dest));
                flip(colorPiecesBB[Color::WHITE], Square::north(dest));

                cpiece[from]= PieceType::Pawn;
                cpiece[dest]  = PieceType::NO_TYPE;
                cpiece[Square::north(dest)] = PieceType::Pawn;
            }
        }

        //====================================================================================
        //  Roques
        //------------------------------------------------------------------------------------
        else if (Move::is_castling(move))
        {

            //====================================================================================
            //  Petit Roque
            //------------------------------------------------------------------------------------
            if ((square_to_bit(dest)) & FILE_G_BB)
            {
                // Move the rook
                flip2(colorPiecesBB[C],  ksc_castle_rook_from[C], ksc_castle_rook_to[C]);
                flip2(typePiecesBB[PieceType::Rook], ksc_castle_rook_from[C], ksc_castle_rook_to[C]);

                cpiece[from]= piece;
                cpiece[dest]  = PieceType::NO_TYPE;
                cpiece[ksc_castle_rook_from[C]] = PieceType::Rook;
                cpiece[ksc_castle_rook_to[C]]   = PieceType::NO_TYPE;
            }

            //====================================================================================
            //  Grand Roque
            //------------------------------------------------------------------------------------
            else if ((square_to_bit(dest)) & FILE_C_BB)
            {
                // Move the rook
                flip2(colorPiecesBB[C], qsc_castle_rook_from[C], qsc_castle_rook_to[C]);
                flip2(typePiecesBB[PieceType::Rook], qsc_castle_rook_from[C], qsc_castle_rook_to[C]);

                cpiece[from]= piece;
                cpiece[dest]  = PieceType::NO_TYPE;
                cpiece[qsc_castle_rook_from[C]] = PieceType::Rook;
                cpiece[qsc_castle_rook_to[C]]   = PieceType::NO_TYPE;
            }
        }
    }

#ifndef NDEBUG
    valid<C>();
#endif
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
    hash      = my_history[game_clock].hash;
    pawn_hash = my_history[game_clock].pawn_hash;
#endif

    assert(valid<C>());
}



// Explicit instantiations.

template void Board::undo_move<WHITE>() noexcept ;
template void Board::undo_move<BLACK>() noexcept ;

template void Board::undo_nullmove<WHITE>() noexcept ;
template void Board::undo_nullmove<BLACK>() noexcept ;
