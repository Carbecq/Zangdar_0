#include "Board.h"
#include "Square.h"
#include "Move.h"

//=============================================================
//! \brief  Enlève un coup
//-------------------------------------------------------------
template <Color C> constexpr void Board::undo_move() noexcept
{
    // Swap sides
    side_to_move = ~side_to_move;

    gamemove_counter--;
    
    const auto &move    = game_history[gamemove_counter].move;
    constexpr Color Them     = ~C;
    const auto dest     = Move::dest(move);
    const auto from     = Move::from(move);
    const auto piece    = Move::piece(move);
    const auto captured = Move::captured(move);
    const auto promo    = Move::promotion(move);

    // En passant
    // back : Returns a reference to the last element in the vector.
    ep_square = game_history[gamemove_counter].ep_square;

    // Halfmoves
    halfmove_counter = game_history[gamemove_counter].halfmove_counter;

    // Fullmoves
    fullmove_counter -= (C == Color::BLACK);

    // Déplacement du roi
    if (piece == KING)
        x_king[C] = from;

    // Castling
    castling = game_history[gamemove_counter].castling;

#if defined USE_HASH
    hash      = game_history[gamemove_counter].hash;
    pawn_hash = game_history[gamemove_counter].pawn_hash;
#endif

    // Remove piece
    BB::toggle_bit(colorPiecesBB[C],    dest);
    BB::toggle_bit(typePiecesBB[piece],  dest);

    // Add piece
    BB::toggle_bit(colorPiecesBB[C], from);
    BB::toggle_bit(typePiecesBB[piece], from);

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
            pieceOn[from] = piece;
            pieceOn[dest] = NO_TYPE;
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
                BB::toggle_bit(typePiecesBB[PAWN], dest);
                BB::toggle_bit(typePiecesBB[promo], dest);

                // Replace the captured piece
                BB::toggle_bit(typePiecesBB[captured], dest);
                BB::toggle_bit(colorPiecesBB[Them], dest);
                
                pieceOn[from]= PAWN;
                pieceOn[dest]  = captured;
            }

            //====================================================================================
            //  Capture simple
            //------------------------------------------------------------------------------------
            else
            {
                pieceOn[from]= piece;
                BB::toggle_bit(colorPiecesBB[Them], dest);
                BB::toggle_bit(typePiecesBB[captured], dest);
                pieceOn[dest]  = captured;
            }
        }

        //====================================================================================
        //  Promotion simple
        //------------------------------------------------------------------------------------
        else if (Move::is_promoting(move))
        {
            // Replace piece with pawn
            BB::toggle_bit(typePiecesBB[PAWN], dest);
            BB::toggle_bit(typePiecesBB[promo], dest);
            
            pieceOn[from]= PAWN;
            pieceOn[dest]  = NO_TYPE;
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
            pieceOn[from]= piece;
            pieceOn[dest]  = NO_TYPE;
        }

        //====================================================================================
        //  Prise en passant
        //------------------------------------------------------------------------------------
        else if (Move::is_enpassant(move))
        {
            // Replace the captured pawn
            if (C == Color::WHITE)
            {
                BB::toggle_bit(typePiecesBB[PAWN], SQ::south(dest));
                BB::toggle_bit(colorPiecesBB[Color::BLACK], SQ::south(dest));
                
                pieceOn[from]= PAWN;
                pieceOn[dest]  = NO_TYPE;
                pieceOn[SQ::south(dest)] = PAWN;
            }
            else
            {
                BB::toggle_bit(typePiecesBB[PAWN], SQ::north(dest));
                BB::toggle_bit(colorPiecesBB[Color::WHITE], SQ::north(dest));
                
                pieceOn[from]= PAWN;
                pieceOn[dest]  = NO_TYPE;
                pieceOn[SQ::north(dest)] = PAWN;
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
            if ((BB::sq2BB(dest)) & FILE_G_BB)
            {
                // Move the rook
                BB::toggle_bit2(colorPiecesBB[C],  ksc_castle_rook_from[C], ksc_castle_rook_to[C]);
                BB::toggle_bit2(typePiecesBB[ROOK], ksc_castle_rook_from[C], ksc_castle_rook_to[C]);
                
                pieceOn[from]= piece;
                pieceOn[dest]  = NO_TYPE;
                pieceOn[ksc_castle_rook_from[C]] = ROOK;
                pieceOn[ksc_castle_rook_to[C]]   = NO_TYPE;
            }

            //====================================================================================
            //  Grand Roque
            //------------------------------------------------------------------------------------
            else if ((BB::sq2BB(dest)) & FILE_C_BB)
            {
                // Move the rook
                BB::toggle_bit2(colorPiecesBB[C], qsc_castle_rook_from[C], qsc_castle_rook_to[C]);
                BB::toggle_bit2(typePiecesBB[ROOK], qsc_castle_rook_from[C], qsc_castle_rook_to[C]);
                
                pieceOn[from]= piece;
                pieceOn[dest]  = NO_TYPE;
                pieceOn[qsc_castle_rook_from[C]] = ROOK;
                pieceOn[qsc_castle_rook_to[C]]   = NO_TYPE;
            }
        }
    }

#ifndef NDEBUG
    // on ne passe ici qu'en debug
    valid();
#endif
}

//===================================================================
//! \brief  Enlève un NullMove
//-------------------------------------------------------------------
template <Color C> constexpr void Board::undo_nullmove() noexcept
{
    // Swap sides
    side_to_move = ~side_to_move;

    gamemove_counter--;

    // En passant
    ep_square = game_history[gamemove_counter].ep_square;

    // Halfmoves
    halfmove_counter = game_history[gamemove_counter].halfmove_counter;

    // Fullmoves
    fullmove_counter -= (C == Color::BLACK);

    // Castling
    castling = game_history[gamemove_counter].castling;

#if defined USE_HASH
    hash      = game_history[gamemove_counter].hash;
    pawn_hash = game_history[gamemove_counter].pawn_hash;
#endif

#ifndef NDEBUG
    // on ne passe ici qu'en debug
    valid();
#endif
}



// Explicit instantiations.

template void Board::undo_move<WHITE>() noexcept ;
template void Board::undo_move<BLACK>() noexcept ;

template void Board::undo_nullmove<WHITE>() noexcept ;
template void Board::undo_nullmove<BLACK>() noexcept ;
