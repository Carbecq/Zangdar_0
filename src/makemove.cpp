#include "Board.h"
#include "Square.h"
#include <iostream>

#ifndef NDEBUG
#include "MoveGen.h"
#endif

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


template <Color C> constexpr void Board::make_move(const U32 move) noexcept
{
    const auto them     = !C;
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

    assert(cpiece[x_king[C]] != PieceType::NO_TYPE);
    assert(dest != from);
    assert(piece != PieceType::NO_TYPE);
    assert(captured != PieceType::King);
    assert(promo != PieceType::Pawn);
    assert(promo != PieceType::King);
    assert(cpiece[from] == piece);

    // Sauvegarde des caractéristiques de la position
    my_history[game_clock] = UndoInfo{hash, pawn_hash, move, ep_square, halfmove_clock, castling} ;

// La prise en passant n'est valable que tout de suite
// Il faut donc la supprimer
#ifdef HASH
    if (ep_square != NO_SQUARE) {
        hash ^= ep_key[ep_square];
    }
#endif

    // Remove ep
    ep_square = NO_SQUARE;

    // Déplacement du roi
    if (piece == PieceType::King)
        x_king[C] = dest;

// Droit au roque, remove ancient value
#ifdef HASH
    hash ^= castle_key[castling];
#endif

    // Castling permissions
    castling = castling & castle_mask[from] & castle_mask[dest];

// Droit au roque; add new value
#ifdef HASH
    hash ^= castle_key[castling];
#endif

    // Increment halfmove clock
    halfmove_clock++;

    // Fullmoves
    fullmove_clock += (C == Color::BLACK);

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
            flip2(colorPiecesBB[C], from, dest);
            flip2(typePiecesBB[piece], from, dest);

            cpiece[from] = PieceType::NO_TYPE;
            cpiece[dest] = piece;

#ifdef HASH
            hash ^= piece_key[C][piece][from] ^ piece_key[C][piece][dest];
#endif

            assert(captured == PieceType::NO_TYPE);
            assert(promo == PieceType::NO_TYPE);

            if (piece == PieceType::Pawn)
            {
                halfmove_clock = 0;
#ifdef HASH
                pawn_hash ^= piece_key[C][PieceType::Pawn][from] ^ piece_key[C][PieceType::Pawn][dest];
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
                flip(typePiecesBB[PieceType::Pawn], from);
                flip(colorPiecesBB[C], from);

                // Remove the captured piece
                flip(typePiecesBB[captured], dest);
                flip(colorPiecesBB[them], dest);

                // Ajoute la pièce promue
                flip(typePiecesBB[promo], dest);
                flip(colorPiecesBB[C], dest);

                cpiece[from] = PieceType::NO_TYPE;
                cpiece[dest] = promo;

#ifdef HASH
                hash ^= piece_key[C][piece][from];
                hash ^= piece_key[C][promo][dest];
                hash ^= piece_key[them][captured][dest];

                pawn_hash ^= piece_key[C][PieceType::Pawn][from];
#endif
                assert(piece == PieceType::Pawn);
                assert(captured != PieceType::NO_TYPE);
                assert(promo != PieceType::NO_TYPE);
                assert(Square::file(dest) != Square::file(from));
                assert((C == Color::WHITE && Square::rank(dest) == 7) ||
                       (C == Color::BLACK && Square::rank(dest) == 0));
                assert((C == Color::WHITE && Square::rank(from) == 6) ||
                       (C == Color::BLACK && Square::rank(from) == 1));

                halfmove_clock = 0;
            }

            //====================================================================================
            //  Capture simple
            //------------------------------------------------------------------------------------
            else
            {
                flip2(colorPiecesBB[C],   from, dest);
                flip2(typePiecesBB[piece], from, dest);

                cpiece[from] = PieceType::NO_TYPE;
                cpiece[dest] = piece;

#ifdef HASH
                hash ^= piece_key[C][piece][from] ^ piece_key[C][piece][dest];

                if (piece == PieceType::Pawn)
                    pawn_hash ^= piece_key[C][PieceType::Pawn][from] ^ piece_key[C][PieceType::Pawn][dest];
#endif

                assert(captured != PieceType::NO_TYPE);

                flip(colorPiecesBB[them], dest);
                flip(typePiecesBB[captured], dest);

                halfmove_clock = 0;

#ifdef HASH
                hash ^= piece_key[them][captured][dest];

                if (captured == PieceType::Pawn)
                    pawn_hash ^= piece_key[them][PieceType::Pawn][dest];
#endif
                assert(promo == PieceType::NO_TYPE);
            }
        }

        //====================================================================================
        //  Promotion simple
        //------------------------------------------------------------------------------------
        else if (Move::is_promoting(move))
        {
            // suppression du pion
            flip(typePiecesBB[PieceType::Pawn], from);
            flip(colorPiecesBB[C], from);

            // Ajoute la pièce promue
            flip(typePiecesBB[promo], dest);
            flip(colorPiecesBB[C], dest);

            cpiece[from] = PieceType::NO_TYPE;
            cpiece[dest] = promo;

#ifdef HASH
            hash ^= piece_key[C][piece][from];
            hash ^= piece_key[C][promo][dest];

            pawn_hash ^= piece_key[C][PieceType::Pawn][from];
#endif
            assert(piece == PieceType::Pawn);
            assert(captured == PieceType::NO_TYPE);
            assert(promo != PieceType::NO_TYPE);
            assert(Square::file(dest) == Square::file(from));

            assert((C == Color::WHITE && Square::rank(dest) == 7) ||
                   (C == Color::BLACK && Square::rank(dest) == 0));
            assert((C == Color::WHITE && Square::rank(from) == 6) ||
                   (C == Color::BLACK && Square::rank(from) == 1));

            halfmove_clock = 0;

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
            flip2(colorPiecesBB[C], from, dest);
            flip2(typePiecesBB[piece], from, dest);

            cpiece[from] = PieceType::NO_TYPE;
            cpiece[dest] = piece;

#ifdef HASH
            hash      ^= piece_key[C][PieceType::Pawn][from] ^ piece_key[C][PieceType::Pawn][dest];
            pawn_hash ^= piece_key[C][PieceType::Pawn][from] ^ piece_key[C][PieceType::Pawn][dest];
#endif
            assert(piece == PieceType::Pawn);
            assert(captured == PieceType::NO_TYPE);
            assert(promo == PieceType::NO_TYPE);
            assert(Square::file(dest) == Square::file(from));
            assert((C == Color::WHITE && Square::rank(dest) == 3) || (C == Color::BLACK && Square::rank(dest) == 4));
            assert((C == Color::WHITE && Square::rank(from) == 1) || (C == Color::BLACK && Square::rank(from) == 6));

            halfmove_clock = 0;
            ep_square = (C == Color::WHITE) ? Square::south(dest) : Square::north(dest);

#ifdef HASH
            hash ^= ep_key[ep_square];
#endif
        }

        //====================================================================================
        //  Prise en passant
        //------------------------------------------------------------------------------------
        else if (Move::is_enpassant(move))
        {
            flip2(colorPiecesBB[C],   from, dest);
            flip2(typePiecesBB[PieceType::Pawn], from, dest);

            cpiece[from] = PieceType::NO_TYPE;
            cpiece[dest] = PieceType::Pawn;

#ifdef HASH
            hash      ^= piece_key[C][PieceType::Pawn][from] ^ piece_key[C][PieceType::Pawn][dest];
            pawn_hash ^= piece_key[C][PieceType::Pawn][from] ^ piece_key[C][PieceType::Pawn][dest];
#endif
            assert(piece == PieceType::Pawn);
         //   assert(captured == PieceType::Pawn);
            assert(promo == PieceType::NO_TYPE);
            assert(Square::file(dest) == Square::file(ep_old));
            assert((C == Color::WHITE && Square::rank(dest) == 5)   || (C == Color::BLACK && Square::rank(dest) == 2));
            assert((C == Color::WHITE && Square::rank(from) == 4) || (C == Color::BLACK && Square::rank(from) == 3));
            assert(Square::file(dest) - Square::file(from) == 1     || Square::file(from) - Square::file(dest) == 1);

            halfmove_clock = 0;

            // Remove the captured pawn
            if (C == Color::WHITE)
            {
                flip(typePiecesBB[PieceType::Pawn], Square::south(dest));
                flip(colorPiecesBB[Color::BLACK],   Square::south(dest));
                cpiece[Square::south(dest)] = PieceType::NO_TYPE;

#ifdef HASH
                hash      ^= piece_key[them][PieceType::Pawn][Square::south(dest)];
                pawn_hash ^= piece_key[them][PieceType::Pawn][Square::south(dest)];
#endif
            }
            else
            {
                flip(typePiecesBB[PieceType::Pawn], Square::north(dest));
                flip(colorPiecesBB[Color::WHITE],   Square::north(dest));
                cpiece[Square::north(dest)] = PieceType::NO_TYPE;

#ifdef HASH
                hash      ^= piece_key[them][PieceType::Pawn][Square::north(dest)];
                pawn_hash ^= piece_key[them][PieceType::Pawn][Square::north(dest)];
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
            if ((square_to_bit(dest)) & FILE_G_BB)
            {
                assert(piece == PieceType::King);
                assert(captured == PieceType::NO_TYPE);
                assert(promo == PieceType::NO_TYPE);
                assert(dest == ksc_castle_king_to[C]);
                assert(cpiece[from] == PieceType::King);
                assert(cpiece[dest]   == PieceType::NO_TYPE);

                flip2(colorPiecesBB[C], from, dest);
                flip2(typePiecesBB[piece], from, dest);

                cpiece[from] = PieceType::NO_TYPE;
                cpiece[dest] = piece;

                assert(cpiece[from] == PieceType::NO_TYPE);
                assert(cpiece[dest] == PieceType::King);

#ifdef HASH
                hash ^= piece_key[C][piece][from];
                hash ^= piece_key[C][piece][dest];
                hash ^= piece_key[C][PieceType::Rook][ksc_castle_rook_from[C]];
                hash ^= piece_key[C][PieceType::Rook][ksc_castle_rook_to[C]];
#endif

                // Move the rook
                flip2(colorPiecesBB[C], ksc_castle_rook_from[C], ksc_castle_rook_to[C]);
                flip2(typePiecesBB[PieceType::Rook], ksc_castle_rook_from[C], ksc_castle_rook_to[C]);

                cpiece[ksc_castle_rook_from[C]] = PieceType::NO_TYPE;
                cpiece[ksc_castle_rook_to[C]]   = PieceType::Rook;

                // Check if rook is at destination
                assert(cpiece[ksc_castle_rook_to[C]] == PieceType::Rook);
                // Check that king is on its destination square
                assert(cpiece[ksc_castle_king_to[C]] == PieceType::King);
            }

            //====================================================================================
            //  Grand Roque
            //------------------------------------------------------------------------------------
            else if ((square_to_bit(dest)) & FILE_C_BB)
            {
                assert(piece == PieceType::King);
                assert(captured == PieceType::NO_TYPE);
                assert(promo == PieceType::NO_TYPE);
                assert(dest == qsc_castle_king_to[C]);
                assert(cpiece[from] == PieceType::King);
                assert(cpiece[dest] == PieceType::NO_TYPE);

                flip2(colorPiecesBB[C], from, dest);
                flip2(typePiecesBB[piece], from, dest);

                cpiece[from] = PieceType::NO_TYPE;
                cpiece[dest] = piece;

                assert(cpiece[from] == PieceType::NO_TYPE);
                assert(cpiece[dest] == PieceType::King);

#ifdef HASH
                hash ^= piece_key[C][piece][from];
                hash ^= piece_key[C][piece][dest];
                hash ^= piece_key[C][PieceType::Rook][qsc_castle_rook_from[C]];
                hash ^= piece_key[C][PieceType::Rook][qsc_castle_rook_to[C]];
#endif

                // Move the rook
                flip2(colorPiecesBB[C], qsc_castle_rook_from[C], qsc_castle_rook_to[C]);
                flip2(typePiecesBB[PieceType::Rook], qsc_castle_rook_from[C], qsc_castle_rook_to[C]);
                cpiece[qsc_castle_rook_from[C]] = PieceType::NO_TYPE;
                cpiece[qsc_castle_rook_to[C]]   = PieceType::Rook;

                // Check if rook is at destination
                assert(piece_on(qsc_castle_rook_to[C]) == PieceType::Rook);
                // Check that king is on its destination square
                assert(piece_on(qsc_castle_king_to[C]) == PieceType::King);
            }
        } // Roques
    } // Special

    // Swap sides
    side_to_move = ~side_to_move;

    game_clock++;

#ifdef HASH
    hash ^= side_key;

#ifdef DEBUG_HASH
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

//    printf("makemove fin \n"); fflush(stdout);
//    PrintBB( colorPiecesBB[WHITE]);
//    PrintBB( colorPiecesBB[BLACK]);
#ifndef NDEBUG
    valid<C>();
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
    my_history[game_clock] = UndoInfo{hash, pawn_hash, 0, ep_square, halfmove_clock, castling};

// La prise en passant n'est valable que tout de suite
// Il faut donc la supprimer
#ifdef HASH
    if (ep_square != NO_SQUARE) {
        hash ^= ep_key[ep_square];
    }
#endif

    // Remove ep
    ep_square = NO_SQUARE;

    // Increment halfmove clock
    halfmove_clock++;

    // Fullmoves
    fullmove_clock += (C == Color::BLACK);

    // Swap sides
    side_to_move = ~side_to_move;

    game_clock++;

#ifdef HASH
    hash ^= side_key;
#endif

    assert(valid<C>());
}

// Explicit instantiations.
template void Board::make_move<WHITE>(const U32 move) noexcept;
template void Board::make_move<BLACK>(const U32 move) noexcept;

template void Board::make_nullmove<WHITE>() noexcept;
template void Board::make_nullmove<BLACK>() noexcept;
