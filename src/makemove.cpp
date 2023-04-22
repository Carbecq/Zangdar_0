#include "Board.h"
#include "Square.h"
#include <iostream>

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
    const auto to       = Move::dest(move);
    const auto from     = Move::from(move);
    const auto piece    = Move::piece(move);
    const auto captured = Move::captured(move);
    const auto promo    = Move::promotion(move);
#ifndef NDEBUG
    // on ne passe ici qu'en debug
    const auto ep_old   = ep_square;
#endif

    assert(cpiece[x_king[C]] != PieceType::NO_TYPE);
    assert(to != from);
    assert(piece != PieceType::NO_TYPE);
    assert(captured != PieceType::King);
    assert(promo != PieceType::Pawn);
    assert(promo != PieceType::King);
    assert(cpiece[from] == piece);

    // Sauvegarde des caractéristiques de la position
    my_history[game_clock] = UndoInfo{hash, move, ep_square, halfmove_clock, castling} ;

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
        x_king[C] = to;

    // Droit au roque, remove ancient value
#ifdef HASH
    hash ^= castle_key[castling];
#endif

    // Castling permissions
    //TODO peut-on le mettre ailleurs , de façon à ne le faire que si c'est utile ?
    // update castling rights
    castling = castling & castle_mask[from] & castle_mask[to];

    // Droit au roque; add new value
#ifdef HASH
    hash ^= castle_key[castling];
#endif

    // Increment halfmove clock
    halfmove_clock++;

    // Fullmoves
    fullmove_clock += (C == Color::BLACK);

    switch (Move::type(move))
    {
    //====================================================================================
    case MoveType::Normal:
        flip2(colorPiecesBB[C], from, to);
        flip2(typePiecesBB[piece], from, to);

        cpiece[from] = PieceType::NO_TYPE;
        cpiece[to]   = piece;

#ifdef HASH
        hash ^= piece_key[C][piece][from];
        hash ^= piece_key[C][piece][to];
#endif

        assert(captured == PieceType::NO_TYPE);
        assert(promo == PieceType::NO_TYPE);

        if (piece == PieceType::Pawn)
        {
            halfmove_clock = 0;
        }
        break;

        //====================================================================================
    case MoveType::Capture:
        flip2(colorPiecesBB[C],   from, to);
        flip2(typePiecesBB[piece], from, to);

        cpiece[from] = PieceType::NO_TYPE;
        cpiece[to]   = piece;

#ifdef HASH
        hash ^= piece_key[C][piece][from];
        hash ^= piece_key[C][piece][to];
#endif

        assert(captured != PieceType::NO_TYPE);

        flip(colorPiecesBB[them], to);
        flip(typePiecesBB[captured], to);

        halfmove_clock = 0;

#ifdef HASH
        hash ^= piece_key[them][captured][to];
#endif
        assert(promo == PieceType::NO_TYPE);

        break;

        //====================================================================================
    case MoveType::Double:
        flip2(colorPiecesBB[C], from, to);
        flip2(typePiecesBB[piece], from, to);

        cpiece[from] = PieceType::NO_TYPE;
        cpiece[to]   = piece;

#ifdef HASH
        hash ^= piece_key[C][piece][from];
        hash ^= piece_key[C][piece][to];
#endif
        assert(piece == PieceType::Pawn);
        assert(captured == PieceType::NO_TYPE);
        assert(promo == PieceType::NO_TYPE);
        assert(Square::file(to) == Square::file(from));
        assert((C == Color::WHITE && Square::rank(to) == 3) || (C == Color::BLACK && Square::rank(to) == 4));
        assert((C == Color::WHITE && Square::rank(from) == 1) || (C == Color::BLACK && Square::rank(from) == 6));

        halfmove_clock = 0;
        ep_square = (C == Color::WHITE) ? Square::south(to) : Square::north(to);

#ifdef HASH
        hash ^= ep_key[ep_square];
#endif
        break;

        //====================================================================================
    case MoveType::EnPassant:
        flip2(colorPiecesBB[C],   from, to);
        flip2(typePiecesBB[piece], from, to);

        cpiece[from] = PieceType::NO_TYPE;
        cpiece[to]   = piece;

#ifdef HASH
        hash ^= piece_key[C][piece][from];
        hash ^= piece_key[C][piece][to];
#endif
        assert(piece == PieceType::Pawn);
        assert(captured == PieceType::Pawn);
        assert(promo == PieceType::NO_TYPE);
        assert(Square::file(to) == Square::file(ep_old));
        assert((C == Color::WHITE && Square::rank(to) == 5) ||
               (C == Color::BLACK && Square::rank(to) == 2));
        assert((C == Color::WHITE && Square::rank(from) == 4) ||
               (C == Color::BLACK && Square::rank(from) == 3));
        assert(Square::file(to) - Square::file(from) == 1 ||
               Square::file(from) - Square::file(to) == 1);

        halfmove_clock = 0;

        // Remove the captured pawn
        if (C == Color::WHITE)
        {
            flip(typePiecesBB[PieceType::Pawn], Square::south(to));
            flip(colorPiecesBB[Color::BLACK],    Square::south(to));
            cpiece[Square::south(to)] = PieceType::NO_TYPE;

#ifdef HASH
            hash ^= piece_key[them][PieceType::Pawn][Square::south(to)];
#endif
        }
        else
        {
            flip(typePiecesBB[PieceType::Pawn], Square::north(to));
            flip(colorPiecesBB[Color::WHITE],    Square::north(to));
            cpiece[Square::north(to)] = PieceType::NO_TYPE;

#ifdef HASH
            hash ^= piece_key[them][PieceType::Pawn][Square::north(to)];
#endif
        }
        break;

        //====================================================================================
    case MoveType::KingCastle:
        assert(piece == PieceType::King);
        assert(captured == PieceType::NO_TYPE);
        assert(promo == PieceType::NO_TYPE);
        assert(to == ksc_castle_king_to[C]);
        assert(cpiece[from] == PieceType::King);
        assert(cpiece[to]   == PieceType::NO_TYPE);

        flip2(colorPiecesBB[C], from, to);
        flip2(typePiecesBB[piece], from, to);

        cpiece[from] = PieceType::NO_TYPE;
        cpiece[to]   = piece;

        assert(cpiece[from] == PieceType::NO_TYPE);
        assert(cpiece[to]   == PieceType::King);


#ifdef HASH
        hash ^= piece_key[C][piece][from];
        hash ^= piece_key[C][piece][to];
        hash ^= piece_key[C][PieceType::Rook][ksc_castle_rook_from[C]];
        hash ^= piece_key[C][PieceType::Rook][ksc_castle_rook_to[C]];
#endif

        // Move the rook
        flip2(colorPiecesBB[C], ksc_castle_rook_from[C], ksc_castle_rook_to[C]);
        flip2(typePiecesBB[PieceType::Rook], ksc_castle_rook_from[C], ksc_castle_rook_to[C]);

        cpiece[ksc_castle_rook_from[C]] = PieceType::NO_TYPE;
        cpiece[ksc_castle_rook_to[C]]   = PieceType::Rook;


        // No overlap between any pieces and the path of the king, exclude the castling rook
        //        assert(!(occupied() & squares_between(from, castle_king_to[us * 2]) & ~Bitboard(ksc_rook_to[C])));
        // No overlap between any pieces and the path of the rook, exclude the castled king
        //        assert(
        //                    !(occupied() & squares_between(castle_rooks_from_[us * 2], ksc_rook_to[C]) & ~occupancy(PieceType::King)));

        // Check if rook is at destination
        assert(cpiece[ksc_castle_rook_to[C]] == PieceType::Rook);
        // Check that king is on its destination square
        assert(cpiece[ksc_castle_king_to[C]] == PieceType::King);

        // Start square of king is either empty, its own, or the rook's target square
        //      assert(piece_on(from) == PieceType::NO_TYPE || from == ksc_rook_to[C] || from == castle_king_to[us * 2]);
        // Start square of rook is either empty, its own, or the king's target square
        //        assert(piece_on(castle_rooks_from_[us * 2]) == PieceType::NO_TYPE ||
        //                castle_rooks_from_[us * 2] == ksc_rook_to[C] ||
        //                castle_rooks_from_[us * 2] == castle_king_to[us * 2]);

        // Check if all squares touched by king are not attacked
        //        assert(!(squares_attacked(them) &
        //                 (squares_between(from, castle_king_to[us * 2]) | from | pieces(us, PieceType::King))));

        break;

        //====================================================================================
    case MoveType::QueenCastle:
        assert(piece == PieceType::King);
        assert(captured == PieceType::NO_TYPE);
        assert(promo == PieceType::NO_TYPE);
        assert(to == qsc_castle_king_to[C]);
        assert(cpiece[from] == PieceType::King);
        assert(cpiece[to]   == PieceType::NO_TYPE);

        flip2(colorPiecesBB[C], from, to);
        flip2(typePiecesBB[piece], from, to);

        cpiece[from] = PieceType::NO_TYPE;
        cpiece[to]   = piece;

        assert(cpiece[from] == PieceType::NO_TYPE);
        assert(cpiece[to]   == PieceType::King);

#ifdef HASH
        hash ^= piece_key[C][piece][from];
        hash ^= piece_key[C][piece][to];
        hash ^= piece_key[C][PieceType::Rook][qsc_castle_rook_from[C]];
        hash ^= piece_key[C][PieceType::Rook][qsc_castle_rook_to[C]];
#endif

        // Move the rook
        flip2(colorPiecesBB[C], qsc_castle_rook_from[C], qsc_castle_rook_to[C]);
        flip2(typePiecesBB[PieceType::Rook], qsc_castle_rook_from[C], qsc_castle_rook_to[C]);
        cpiece[qsc_castle_rook_from[C]] = PieceType::NO_TYPE;
        cpiece[qsc_castle_rook_to[C]]   = PieceType::Rook;

        // No overlap between any pieces and the path of the king, exclude the castling rook
        //       assert(!(occupied() & squares_between(from, castle_king_to[us * 2 + 1]) & ~Bitboard(qsc_rook_to[C])));
        // No overlap between any pieces and the path of the rook, exclude the castled king
        //        assert(!(occupied() & squares_between(castle_rooks_from_[us * 2 + 1], qsc_rook_to[C]) &
        //               ~occupancy(PieceType::King)));

        // Check if rook is at destination
        //        assert(piece_on(qsc_rook_to[C]) == PieceType::Rook);
        // Check that king is on its destination square
        //        assert(piece_on(castle_king_to[us * 2 + 1]) == PieceType::King);
        //        assert(castle_king_to[us * 2 + 1] == pieces(us, PieceType::King).hsb());

        // Start square of rook is either empty, its own, or the king's target square
        //ZZ        assert(piece_on(castle_rooks_from_[us * 2 + 1]) == PieceType::NO_TYPE ||
        //                castle_rooks_from_[us * 2 + 1] == qsc_rook_to[C] ||
        //                castle_rooks_from_[us * 2 + 1] == castle_king_to[us * 2 + 1]);
        // Start square of king is either empty, its own, or the rook's target square
        //        assert(piece_on(from) == PieceType::NO_TYPE || from == qsc_rook_to[C] || from == castle_king_to[us * 2 + 1]);

        // Check if all squares touched by king are not attacked
        //        assert(!(squares_attacked(them) &
        //                 (squares_between(from, castle_king_to[us * 2 + 1]) | from | pieces(us, PieceType::King))));

        break;

        //====================================================================================
    case MoveType::Promotion:

        // suppression du pion
        flip(typePiecesBB[PieceType::Pawn], from);
        flip(colorPiecesBB[C], from);

        // Ajoute la pièce promue
        flip(typePiecesBB[promo], to);
        flip(colorPiecesBB[C], to);

        cpiece[from] = PieceType::NO_TYPE;
        cpiece[to]   = promo;

#ifdef HASH
        hash ^= piece_key[C][piece][from];
        hash ^= piece_key[C][promo][to];
#endif
        assert(piece == PieceType::Pawn);
        assert(captured == PieceType::NO_TYPE);
        assert(promo != PieceType::NO_TYPE);
        assert(Square::file(to) == Square::file(from));

        assert((C == Color::WHITE && Square::rank(to) == 7) ||
               (C == Color::BLACK && Square::rank(to) == 0));
        assert((C == Color::WHITE && Square::rank(from) == 6) ||
               (C == Color::BLACK && Square::rank(from) == 1));

        halfmove_clock = 0;

        break;

        //====================================================================================
    case MoveType::PromotionCapture:

        // suppression du pion
        flip(typePiecesBB[PieceType::Pawn], from);
        flip(colorPiecesBB[C], from);

        // Remove the captured piece
        flip(typePiecesBB[captured], to);
        flip(colorPiecesBB[them], to);

        // Ajoute la pièce promue
        flip(typePiecesBB[promo], to);
        flip(colorPiecesBB[C], to);

        cpiece[from] = PieceType::NO_TYPE;
        cpiece[to]   = promo;

#ifdef HASH
        hash ^= piece_key[C][piece][from];
        hash ^= piece_key[C][promo][to];
        hash ^= piece_key[them][captured][to];
#endif
        assert(piece == PieceType::Pawn);
        assert(captured != PieceType::NO_TYPE);
        assert(promo != PieceType::NO_TYPE);
        assert(Square::file(to) != Square::file(from));
        assert((C == Color::WHITE && Square::rank(to) == 7) ||
               (C == Color::BLACK && Square::rank(to) == 0));
        assert((C == Color::WHITE && Square::rank(from) == 6) ||
               (C == Color::BLACK && Square::rank(from) == 1));

        halfmove_clock = 0;

        break;

        //====================================================================================
    default:
        abort();
    }

    // Swap sides
    side_to_move = ~side_to_move;

    game_clock++;

#ifdef HASH
    hash ^= side_key;

#ifdef DEBUG_HASH
    U64 test_hash = calculate_hash();

    if (test_hash != hash)
    {
        std::cout << Move::name(move) << "  : hash pb : hash = " << hash << " ; calc = " << test_hash << std::endl;
        std::cout << display() << std::endl << std::endl;
    }
#endif
#endif

    //    printf("makemove fin \n"); fflush(stdout);
    //    PrintBB( colorPiecesBB[WHITE]);
    //    PrintBB( colorPiecesBB[BLACK]);

    assert(valid<C>());
}


//=======================================================================
//! \brief un Null Move est un coup où on passe son tour.
//! la position ne change pas, excepté le camp qui change.
//-----------------------------------------------------------------------
template <Color C> constexpr void Board::make_nullmove() noexcept
{
    // Sauvegarde des caractéristiques de la position
    // NullMove = 0
    my_history[game_clock] = UndoInfo{hash, 0, ep_square, halfmove_clock, castling};

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
