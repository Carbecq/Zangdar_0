#include "Board.h"
#include "Square.h"
#include "Move.h"

/* This is the castle_mask array. We can use it to determine
the castling permissions after a move. What we do is
logical-AND the castle bits with the castle_mask bits for
both of the move's ints. Let's say castle is 1, meaning
that white can still castle kingside. Now we play a move
where the rook on h1 gets captured. We AND castle with
castle_mask[63], so we have 1&14, and castle becomes 0 and
white can't castle kingside anymore.
 (TSCP) */

constexpr U32 castle_mask[64] = {
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    7, 15, 15, 15,  3, 15, 15, 11
};


template <Color C> constexpr void Board::make_move(const MOVE move) noexcept
{
    constexpr Color Them     = ~C;
    const auto dest     = Move::dest(move);
    const auto from     = Move::from(move);
    const auto piece    = Move::piece(move);
    const auto captured = Move::captured(move);
    const auto promo    = Move::promotion(move);
#ifndef NDEBUG
    // on ne passe ici qu'en debug
    const auto ep_old   = ep_square;
#endif

    //    printf("move  = (%s) \n", Move::name(move).c_str());
    //    binary_print(move);
    //    printf("side  = %s \n", side_name[C].c_str());
    //    printf("from  = %s \n", square_name[from].c_str());
    //    printf("dest  = %s \n", square_name[dest].c_str());
    //    printf("piece = %s \n", piece_name[piece].c_str());
    //    printf("capt  = %s \n", piece_name[captured].c_str());
    //    printf("promo = %s \n", piece_name[promo].c_str());
    //    printf("flags = %u \n", Move::flags(move));

    assert(pieceOn[king_square<C>()] == KING);
    assert(dest != from);
    assert(piece != NO_TYPE);
    assert(captured != KING);
    assert(promo != PAWN);
    assert(promo != KING);
    assert(pieceOn[from] == piece);

    // Sauvegarde des caractéristiques de la position
    game_history[gamemove_counter] = UndoInfo{hash, pawn_hash, move, ep_square, halfmove_counter, castling} ;

// La prise en passant n'est valable que tout de suite
// Il faut donc la supprimer
#if defined USE_HASH
    if (ep_square != NO_SQUARE) {
        hash ^= ep_key[ep_square];
    }
#endif

    // Remove ep
    ep_square = NO_SQUARE;

    // Déplacement du roi
    if (piece == KING)
        x_king[C] = dest;

// Droit au roque, remove ancient value
#if defined USE_HASH
    hash ^= castle_key[castling];
#endif

    // Castling permissions
    castling = castling & castle_mask[from] & castle_mask[dest];

// Droit au roque; add new value
#if defined USE_HASH
    hash ^= castle_key[castling];
#endif

    // Increment halfmove clock
    halfmove_counter++;

    // Fullmoves
    fullmove_counter += (C == Color::BLACK);

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
            BB::toggle_bit2(colorPiecesBB[C], from, dest);
            BB::toggle_bit2(typePiecesBB[piece], from, dest);
            
            pieceOn[from] = NO_TYPE;
            pieceOn[dest] = piece;

#if defined USE_HASH
            hash ^= piece_key[C][piece][from] ^ piece_key[C][piece][dest];
#endif

            assert(captured == NO_TYPE);
            assert(promo == NO_TYPE);

            if (piece == PAWN)
            {
                halfmove_counter = 0;
#if defined USE_HASH
                pawn_hash ^= piece_key[C][PAWN][from] ^ piece_key[C][PAWN][dest];
#endif
            }
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
                // suppression du pion
                BB::toggle_bit(typePiecesBB[PAWN], from);
                BB::toggle_bit(colorPiecesBB[C], from);

                // Remove the captured piece
                BB::toggle_bit(typePiecesBB[captured], dest);
                BB::toggle_bit(colorPiecesBB[Them], dest);

                // Ajoute la pièce promue
                BB::toggle_bit(typePiecesBB[promo], dest);
                BB::toggle_bit(colorPiecesBB[C], dest);
                
                pieceOn[from] = NO_TYPE;
                pieceOn[dest] = promo;

#if defined USE_HASH
                hash ^= piece_key[C][piece][from];
                hash ^= piece_key[C][promo][dest];
                hash ^= piece_key[Them][captured][dest];

                pawn_hash ^= piece_key[C][PAWN][from];
#endif
                assert(piece == PAWN);
                assert(captured != NO_TYPE);
                assert(promo != NO_TYPE);
                assert(SQ::file(dest) != SQ::file(from));
                assert((C == Color::WHITE && SQ::rank(dest) == 7) ||
                       (C == Color::BLACK && SQ::rank(dest) == 0));
                assert((C == Color::WHITE && SQ::rank(from) == 6) ||
                       (C == Color::BLACK && SQ::rank(from) == 1));

                halfmove_counter = 0;
            }

            //====================================================================================
            //  Capture simple
            //------------------------------------------------------------------------------------
            else
            {
                BB::toggle_bit2(colorPiecesBB[C],   from, dest);
                BB::toggle_bit2(typePiecesBB[piece], from, dest);
                
                pieceOn[from] = NO_TYPE;
                pieceOn[dest] = piece;

#if defined USE_HASH
                hash ^= piece_key[C][piece][from] ^ piece_key[C][piece][dest];

                if (piece == PAWN)
                    pawn_hash ^= piece_key[C][PAWN][from] ^ piece_key[C][PAWN][dest];
#endif

                assert(captured != NO_TYPE);
                
                BB::toggle_bit(colorPiecesBB[Them], dest);
                BB::toggle_bit(typePiecesBB[captured], dest);

                halfmove_counter = 0;

#if defined USE_HASH
                hash ^= piece_key[Them][captured][dest];

                if (captured == PAWN)
                    pawn_hash ^= piece_key[Them][PAWN][dest];
#endif
                assert(promo == NO_TYPE);
            }
        }

        //====================================================================================
        //  Promotion simple
        //------------------------------------------------------------------------------------
        else if (Move::is_promoting(move))
        {
            // suppression du pion
            BB::toggle_bit(typePiecesBB[PAWN], from);
            BB::toggle_bit(colorPiecesBB[C], from);

            // Ajoute la pièce promue
            BB::toggle_bit(typePiecesBB[promo], dest);
            BB::toggle_bit(colorPiecesBB[C], dest);
            
            pieceOn[from] = NO_TYPE;
            pieceOn[dest] = promo;

#if defined USE_HASH
            hash ^= piece_key[C][piece][from];
            hash ^= piece_key[C][promo][dest];

            pawn_hash ^= piece_key[C][PAWN][from];
#endif
            assert(piece == PAWN);
            assert(captured == NO_TYPE);
            assert(promo != NO_TYPE);
            assert(SQ::file(dest) == SQ::file(from));

            assert((C == Color::WHITE && SQ::rank(dest) == 7) ||
                   (C == Color::BLACK && SQ::rank(dest) == 0));
            assert((C == Color::WHITE && SQ::rank(from) == 6) ||
                   (C == Color::BLACK && SQ::rank(from) == 1));

            halfmove_counter = 0;

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
            BB::toggle_bit2(colorPiecesBB[C], from, dest);
            BB::toggle_bit2(typePiecesBB[piece], from, dest);
            
            pieceOn[from] = NO_TYPE;
            pieceOn[dest] = piece;

#if defined USE_HASH
            hash      ^= piece_key[C][PAWN][from] ^ piece_key[C][PAWN][dest];
            pawn_hash ^= piece_key[C][PAWN][from] ^ piece_key[C][PAWN][dest];
#endif
            assert(piece == PAWN);
            assert(captured == NO_TYPE);
            assert(promo == NO_TYPE);
            assert(SQ::file(dest) == SQ::file(from));
            assert((C == Color::WHITE && SQ::rank(dest) == 3) || (C == Color::BLACK && SQ::rank(dest) == 4));
            assert((C == Color::WHITE && SQ::rank(from) == 1) || (C == Color::BLACK && SQ::rank(from) == 6));

            halfmove_counter = 0;
            ep_square = (C == Color::WHITE) ? SQ::south(dest) : SQ::north(dest);

#if defined USE_HASH
            hash ^= ep_key[ep_square];
#endif
        }

        //====================================================================================
        //  Prise en passant
        //------------------------------------------------------------------------------------
        else if (Move::is_enpassant(move))
        {
            BB::toggle_bit2(colorPiecesBB[C],   from, dest);
            BB::toggle_bit2(typePiecesBB[PAWN], from, dest);
            
            pieceOn[from] = NO_TYPE;
            pieceOn[dest] = PAWN;

#if defined USE_HASH
            hash      ^= piece_key[C][PAWN][from] ^ piece_key[C][PAWN][dest];
            pawn_hash ^= piece_key[C][PAWN][from] ^ piece_key[C][PAWN][dest];
#endif
            assert(piece == PAWN);
         //   assert(captured == PAWN);
            assert(promo == NO_TYPE);
            assert(SQ::file(dest) == SQ::file(ep_old));
            assert((C == Color::WHITE && SQ::rank(dest) == 5)   || (C == Color::BLACK && SQ::rank(dest) == 2));
            assert((C == Color::WHITE && SQ::rank(from) == 4) || (C == Color::BLACK && SQ::rank(from) == 3));
            assert(SQ::file(dest) - SQ::file(from) == 1     || SQ::file(from) - SQ::file(dest) == 1);

            halfmove_counter = 0;

            // Remove the captured pawn
            if (C == Color::WHITE)
            {
                BB::toggle_bit(typePiecesBB[PAWN], SQ::south(dest));
                BB::toggle_bit(colorPiecesBB[Color::BLACK],   SQ::south(dest));
                pieceOn[SQ::south(dest)] = NO_TYPE;

#if defined USE_HASH
                hash      ^= piece_key[Them][PAWN][SQ::south(dest)];
                pawn_hash ^= piece_key[Them][PAWN][SQ::south(dest)];
#endif
            }
            else
            {
                BB::toggle_bit(typePiecesBB[PAWN], SQ::north(dest));
                BB::toggle_bit(colorPiecesBB[Color::WHITE],   SQ::north(dest));
                pieceOn[SQ::north(dest)] = NO_TYPE;

#if defined USE_HASH
                hash      ^= piece_key[Them][PAWN][SQ::north(dest)];
                pawn_hash ^= piece_key[Them][PAWN][SQ::north(dest)];
#endif
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
                assert(piece == KING);
                assert(captured == NO_TYPE);
                assert(promo == NO_TYPE);
                assert(dest == ksc_castle_king_to[C]);
                assert(pieceOn[from] == KING);
                assert(pieceOn[dest]   == NO_TYPE);

                BB::toggle_bit2(colorPiecesBB[C], from, dest);
                BB::toggle_bit2(typePiecesBB[piece], from, dest);
                
                pieceOn[from] = NO_TYPE;
                pieceOn[dest] = piece;

                assert(pieceOn[from] == NO_TYPE);
                assert(pieceOn[dest] == KING);

#if defined USE_HASH
                hash ^= piece_key[C][piece][from];
                hash ^= piece_key[C][piece][dest];
                hash ^= piece_key[C][ROOK][ksc_castle_rook_from[C]];
                hash ^= piece_key[C][ROOK][ksc_castle_rook_to[C]];
#endif

                // Move the rook
                BB::toggle_bit2(colorPiecesBB[C], ksc_castle_rook_from[C], ksc_castle_rook_to[C]);
                BB::toggle_bit2(typePiecesBB[ROOK], ksc_castle_rook_from[C], ksc_castle_rook_to[C]);
                
                pieceOn[ksc_castle_rook_from[C]] = NO_TYPE;
                pieceOn[ksc_castle_rook_to[C]]   = ROOK;

                // Check if rook is at destination
                assert(pieceOn[ksc_castle_rook_to[C]] == ROOK);
                // Check that king is on its destination square
                assert(pieceOn[ksc_castle_king_to[C]] == KING);
            }

            //====================================================================================
            //  Grand Roque
            //------------------------------------------------------------------------------------
            else if ((BB::sq2BB(dest)) & FILE_C_BB)
            {
                assert(piece == KING);
                assert(captured == NO_TYPE);
                assert(promo == NO_TYPE);
                assert(dest == qsc_castle_king_to[C]);
                assert(pieceOn[from] == KING);
                assert(pieceOn[dest] == NO_TYPE);

                BB::toggle_bit2(colorPiecesBB[C], from, dest);
                BB::toggle_bit2(typePiecesBB[piece], from, dest);
                
                pieceOn[from] = NO_TYPE;
                pieceOn[dest] = piece;

                assert(pieceOn[from] == NO_TYPE);
                assert(pieceOn[dest] == KING);

#if defined USE_HASH
                hash ^= piece_key[C][piece][from];
                hash ^= piece_key[C][piece][dest];
                hash ^= piece_key[C][ROOK][qsc_castle_rook_from[C]];
                hash ^= piece_key[C][ROOK][qsc_castle_rook_to[C]];
#endif

                // Move the rook
                BB::toggle_bit2(colorPiecesBB[C], qsc_castle_rook_from[C], qsc_castle_rook_to[C]);
                BB::toggle_bit2(typePiecesBB[ROOK], qsc_castle_rook_from[C], qsc_castle_rook_to[C]);
                pieceOn[qsc_castle_rook_from[C]] = NO_TYPE;
                pieceOn[qsc_castle_rook_to[C]]   = ROOK;

                // Check if rook is at destination
                assert(piece_on(qsc_castle_rook_to[C]) == ROOK);
                // Check that king is on its destination square
                assert(piece_on(qsc_castle_king_to[C]) == KING);
            }
        } // Roques
    } // Special

    // Swap sides
    side_to_move = ~side_to_move;

    gamemove_counter++;

#if defined USE_HASH
    hash ^= side_key;

#if defined DEBUG_HASH
    U64 hash_1, hash_2;
        calculate_hash(hash_1, hash_2);

    if (hash_1 != hash)
    {
        std::cout << Move::name(move) << "  : hash pb : hash = " << hash << " ; calc = " << hash_1 << std::endl;
        std::cout << display() << std::endl << std::endl;
    }
    if (hash_2 != pawn_hash)
    {
        std::cout << Move::name(move) << "  : pawn_hash pb : pawn_hash = " << pawn_hash << " ; calc = " << hash_2 << std::endl;
        std::cout << display() << std::endl << std::endl;
    }
#endif
#endif

#ifndef NDEBUG
    // on ne passe ici qu'en debug
    assert(valid());
#endif
}


//=======================================================================
//! \brief un Null Move est un coup où on passe son tour.
//! la position ne change pas, excepté le camp qui change.
//-----------------------------------------------------------------------
template <Color C> constexpr void Board::make_nullmove() noexcept
{
    // Sauvegarde des caractéristiques de la position
    // NullMove = 0
    game_history[gamemove_counter] = UndoInfo{hash, pawn_hash, Move::MOVE_NULL, ep_square, halfmove_counter, castling};

// La prise en passant n'est valable que tout de suite
// Il faut donc la supprimer
#if defined USE_HASH
    if (ep_square != NO_SQUARE) {
        hash ^= ep_key[ep_square];
    }
#endif

    // Remove ep
    ep_square = NO_SQUARE;

    // Increment halfmove clock
    halfmove_counter++;

    // Fullmoves
    fullmove_counter += (C == Color::BLACK);

    // Swap sides
    side_to_move = ~side_to_move;

    gamemove_counter++;

#if defined USE_HASH
    hash ^= side_key;
#endif

#ifndef NDEBUG
    // on ne passe ici qu'en debug
    valid();
#endif
}

// Explicit instantiations.
template void Board::make_move<WHITE>(const U32 move) noexcept;
template void Board::make_move<BLACK>(const U32 move) noexcept;

template void Board::make_nullmove<WHITE>() noexcept;
template void Board::make_nullmove<BLACK>() noexcept;
