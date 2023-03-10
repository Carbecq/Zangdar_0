#include "Board.h"
#include "bitboard.h"
#include "movegen.h"

constexpr int PUSH[] = {8, -8};


//=================================================================
//! \brief  Génération de tous les captures légales
//! \param  ml  Liste des coups dans laquelle on va stocker les coups
//!
//! algorithme de Mperft
//-----------------------------------------------------------------
template <Color C>
constexpr void Board::legal_evasions(MoveList& ml) noexcept
{
    const Color O = ~C;
    const int   K = x_king[C];

    const Bitboard occupiedBB = occupied();     // toutes les pièces (Blanches + Noires)
    Bitboard emptyBB    = ~occupiedBB;
    const Bitboard usBB       = colorPiecesBB[C];
    Bitboard enemyBB    = colorPiecesBB[O];

    const Bitboard bq         = typePiecesBB[PieceType::Bishop] | typePiecesBB[PieceType::Queen];
    const Bitboard rq         = typePiecesBB[PieceType::Rook]   | typePiecesBB[PieceType::Queen];

    //-----------------------------------------------------------------------------------------
    //  Calcul des pièces clouées, et des échecs
    //  algorithme de Surge
    //-----------------------------------------------------------------------------------------
// https://github.com/official-stockfish/Stockfish/blob/master/src/position.cpp

    int s;
    Bitboard b1;

    //    const Bitboard our_diag_sliders   = diagonal_sliders<C>();
    const Bitboard their_diag_sliders = diagonal_sliders<~C>();
    //    const Bitboard our_orth_sliders   = orthogonal_sliders<C>();
    const Bitboard their_orth_sliders = orthogonal_sliders<~C>();

    //Checkers of each piece type are identified by:
    //1. Projecting attacks FROM the king square
    //2. Intersecting this bitboard with the enemy bitboard of that piece type
    checkersBB = (movegen::knight_moves(K)    & pieces_cp<~C, PieceType::Knight>() ) |
                 (movegen::pawn_attacks(C, K) & pieces_cp<~C, PieceType::Pawn>() );

    //Here, we identify slider checkers and pinners simultaneously, and candidates for such pinners
    //and checkers are represented by the bitboard <candidates>
    Bitboard candidates = (movegen::rook_moves(K, enemyBB)   & their_orth_sliders) |
                          (movegen::bishop_moves(K, enemyBB) & their_diag_sliders);

    pinnedBB = 0;
    while (candidates)
    {
        s  = next_square(candidates);
        b1 = movegen::squares_between(K, s) & usBB;

        //Do the squares in between the enemy slider and our king contain any of our pieces?
        //If not, add the slider to the checker bitboard
        if (b1 == 0)
            checkersBB |= square_to_bit(s);

        //If there is only one of our pieces between them, add our piece to the pinned bitboard
        else if ((b1 & (b1 - 1)) == 0)
            pinnedBB |= b1;
    }
    //-----------------------------------------------------------------------------------------
    const Bitboard unpinnedBB = colorPiecesBB[C] & ~pinnedBB;

    constexpr int pawn_left = PUSH[C] - 1;
    constexpr int pawn_right = PUSH[C] + 1;
    constexpr int pawn_push = PUSH[C];
    Bitboard pieceBB;
    Bitboard attackBB;
    int from, to, ep;
    int x_checker = NO_SQUARE;

    //-------------------------------------------------------------------------------------------

    ml.clear();

    //    std::cout << "legal_gen 1 ; ch=%d " << Bcount(checkersBB) << std::endl;

    // in check: capture or block the (single) checker if any;
    if (checkersBB)
    {
        if (Bcount(checkersBB) == 1)
        {
            x_checker = first_square(checkersBB);
            emptyBB = movegen::squares_between(K, x_checker);
            enemyBB = checkersBB;
        } else {
            emptyBB = enemyBB  = 0;
        }
    }

    if (ep_square!=NO_SQUARE && (!checkersBB || x_checker == ep_square - pawn_push))
    {
        // file : a...h

        to = ep_square;             // e3 = 20
        ep = to - pawn_push;        // 20 - (-8)  noirs = 28 = e4
        from = ep - 1;              // 28 - 1 = 27 = d4

        Bitboard our_pawns = pieces_cp<C, PieceType::Pawn>();
        if (Square::file(to) > 0 && our_pawns & square_to_bit(from) )
        {
            pieceBB = occupiedBB ^ square_to_bit(from) ^ square_to_bit(ep) ^ square_to_bit(to);

            if ( !(movegen::bishop_moves(K, pieceBB) & bq & colorPiecesBB[O]) &&
                 !(movegen::rook_moves(K, pieceBB)   & rq & colorPiecesBB[O]) )
            {
                add_capture_move(MoveType::enpassant, ml, from, to, PieceType::Pawn, PieceType::Pawn);
            }
        }

        from = ep + 1;          // 28 + 1 = 29 = f4
        if (Square::file(to) < 7 && our_pawns & square_to_bit(from) )
        {
            pieceBB = occupiedBB ^ square_to_bit(from) ^ square_to_bit(ep) ^ square_to_bit(to);

            if ( !(movegen::bishop_moves(K, pieceBB) & bq & colorPiecesBB[O]) &&
                 !(movegen::rook_moves(K, pieceBB)   & rq & colorPiecesBB[O]) )
            {
                add_capture_move(MoveType::enpassant, ml, from, to, PieceType::Pawn, PieceType::Pawn);
            }
        }
    }


    // pawn
    pieceBB = typePiecesBB[Pawn] & unpinnedBB;

    attackBB = (C ? (pieceBB & ~FileMask8[0]) >> 9 : (pieceBB & ~FileMask8[0]) << 7) & enemyBB;
    push_capture_promotions(ml, attackBB & PROMOTION_RANK[C], pawn_left);
    push_pawn_capture_moves(ml, attackBB & ~PROMOTION_RANK[C], pawn_left);

    attackBB = (C ? (pieceBB & ~FileMask8[7]) >> 7 : (pieceBB & ~FileMask8[7]) << 9) & enemyBB;
    push_capture_promotions(ml, attackBB & PROMOTION_RANK[C], pawn_right);
    push_pawn_capture_moves(ml, attackBB & ~PROMOTION_RANK[C], pawn_right);

 //   if (do_quiet)
 //   {
        attackBB = (C ? pieceBB >> 8 : pieceBB << 8) & emptyBB;
        push_quiet_promotions(ml, attackBB & PROMOTION_RANK[C], pawn_push);

        push_pawn_quiet_moves(MoveType::Normal, ml, attackBB & ~PROMOTION_RANK[C], pawn_push);
        attackBB = (C ? (((pieceBB & RankMask8[6]) >> 8) & ~occupiedBB) >> 8 : (((pieceBB & RankMask8[1]) << 8) & ~occupiedBB) << 8) & emptyBB;
        push_pawn_quiet_moves(MoveType::Double, ml, attackBB, 2 * pawn_push);
 //   }

    // knight

    pieceBB = typePiecesBB[Knight] & unpinnedBB;
    while (pieceBB)
    {
        from = next_square(pieceBB);
        attackBB = movegen::knight_moves(from) & enemyBB;
        push_piece_capture_moves(ml, attackBB, from, PieceType::Knight);
        attackBB = movegen::knight_moves(from) & emptyBB;
        push_piece_quiet_moves(ml, attackBB, from, PieceType::Knight);
    }

    // bishop or queen
    pieceBB = bq & unpinnedBB;
    while (pieceBB)
    {
        from = next_square(pieceBB);
        attackBB = movegen::bishop_moves(from, occupiedBB) & enemyBB;
        push_capture_moves(ml, attackBB, from);
        attackBB = movegen::bishop_moves(from, occupiedBB) & emptyBB;
        push_quiet_moves(ml, attackBB, from);
    }

    // rook or queen
    pieceBB = rq & unpinnedBB;
    while (pieceBB)
    {
        from = next_square(pieceBB);
        attackBB = movegen::rook_moves(from, occupiedBB) & enemyBB;
        push_capture_moves(ml, attackBB, from);
        attackBB = movegen::rook_moves(from, occupiedBB) & emptyBB;
        push_quiet_moves(ml, attackBB, from);
    }

    // king
    // on enlève le roi de l'échiquier
    /*
     *   .R...t
     *
     *   dans cette position, si le roi va à gauche, il est toujours attaqué.
     *   si on laisse le roi dans l'échiquier, il ne sera pas attaqué
     */
    colorPiecesBB[C] ^= square_to_bit(K);

    auto mask = movegen::king_moves(K) & colorPiecesBB[O];
    while (mask)
    {
        to = next_square(mask);
        if (!square_attacked<O>(to))
            add_capture_move(MoveType::Capture, ml, K, to, PieceType::King, cpiece[to]);
    }
    mask = movegen::king_moves(K) & ~occupiedBB;
    while (mask)
    {
        to = next_square(mask);
        if (!square_attacked<O>(to))
            add_quiet_move(MoveType::Normal, ml, K, to, PieceType::King);
    }

    // remet le roi dans l'échiquier
    colorPiecesBB[C] ^= square_to_bit(K);
}

// Explicit instantiations.
template void Board::legal_evasions<WHITE>(MoveList& ml) noexcept;
template void Board::legal_evasions<BLACK>(MoveList& ml) noexcept;