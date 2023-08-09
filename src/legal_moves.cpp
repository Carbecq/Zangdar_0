#include "Board.h"
#include "Board.h"
#include "MoveGen.h"
#include "Move.h"

constexpr int PUSH[] = {8, -8};


//==========================================================================
//! \brief  recherche si le coup "token" est un coup légal,
//!         puis l'exécute
//--------------------------------------------------------------------------
template <Color C>
void Board::apply_token(const std::string& token) noexcept
{
    MoveList ml;
    legal_moves<C>(ml);

    for (const auto &move : ml.moves)
    {
        if (Move::name(move) == token)
        {
            make_move<C>(move);
            break;
        }
    }
}

//=================================================================
//! \brief  Génération de tous les captures légales
//! \param  ml  Liste des coups dans laquelle on va stocker les coups
//!
//! algorithme de Mperft
//-----------------------------------------------------------------
template <Color C>
constexpr void Board::legal_moves(MoveList& ml) noexcept
{
    constexpr Color O = ~C;
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

    int s;
    Bitboard b1;

    //    const Bitboard our_diag_sliders   = diagonal_sliders<C>();
    const Bitboard their_diag_sliders = diagonal_sliders<~C>();
    //    const Bitboard our_orth_sliders   = orthogonal_sliders<C>();
    const Bitboard their_orth_sliders = orthogonal_sliders<~C>();

    //Checkers of each piece type are identified by:
    //1. Projecting attacks FROM the king square
    //2. Intersecting this bitboard with the enemy bitboard of that piece type
    Bitboard checkersBB = (MoveGen::knight_moves(K)    & pieces_cp<~C, PieceType::Knight>() ) |
                          (MoveGen::pawn_attacks(C, K) & pieces_cp<~C, PieceType::Pawn>() );

    //Here, we identify slider checkers and pinners simultaneously, and candidates for such pinners
    //and checkers are represented by the bitboard <candidates>
    Bitboard candidates = (MoveGen::rook_moves(K, enemyBB)   & their_orth_sliders) |
                          (MoveGen::bishop_moves(K, enemyBB) & their_diag_sliders);

    Bitboard pinnedBB = 0;
    while (candidates)
    {
        s  = next_square(candidates);
        b1 = MoveGen::squares_between(K, s) & usBB;

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
    const int *dir = allmask[K].direction;
    Bitboard pieceBB;
    Bitboard attackBB;
    int from, to, d, ep;
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
            emptyBB = MoveGen::squares_between(K, x_checker);
            enemyBB = checkersBB;
        } else {
            emptyBB = enemyBB  = 0;
        }
    }
    else
    {
        // not in check: castling & pinned pieces moves

        // castling
        if (can_castle_k<C>())
        {
            /*  MoveGen::squares_between(ksq, ksc_castle_king_to[us])   : case F1
                 *  square_to_bit(ksc_castle_king_to[us])                   : case G1
                 */
            const Bitboard blockers         = occupied() ^ square_to_bit(K) ^ square_to_bit(ksc_castle_rook_from[C]);
            const Bitboard king_path        = (MoveGen::squares_between(K, ksc_castle_king_to[C])  |
                                        square_to_bit(ksc_castle_king_to[C])) ;
            const bool     king_path_clear  = Bempty(king_path & blockers);
            const Bitboard rook_path        = MoveGen::squares_between(ksc_castle_rook_to[C], ksc_castle_rook_from[C])
                                       | square_to_bit(ksc_castle_rook_to[C]);
            const bool     rook_path_clear  = Bempty(rook_path & blockers);

            if (king_path_clear && rook_path_clear && !(squares_attacked<O>() & king_path))
                add_quiet_move(ml, K, ksc_castle_king_to[C], PieceType::King, Move::FLAG_CASTLE);
        }
        if (can_castle_q<C>())
        {
            const Bitboard blockers         = occupied() ^ square_to_bit(K) ^ square_to_bit(qsc_castle_rook_from[C]);
            const Bitboard king_path        = (MoveGen::squares_between(K, qsc_castle_king_to[C]) |
                                        square_to_bit(qsc_castle_king_to[C]));
            const bool     king_path_clear  = Bempty(king_path & blockers);
            const Bitboard rook_path        = MoveGen::squares_between(qsc_castle_rook_to[C], qsc_castle_rook_from[C])
                                       | square_to_bit(qsc_castle_rook_to[C]);
            const bool     rook_path_clear = Bempty(rook_path & blockers);

            if (king_path_clear && rook_path_clear && !(squares_attacked<O>() & king_path))
                add_quiet_move(ml, K, qsc_castle_king_to[C], PieceType::King, Move::FLAG_CASTLE);
        }

        // pawn (pinned)
        pieceBB = typePiecesBB[Pawn] & pinnedBB;

        while (pieceBB) {
            from = next_square(pieceBB);
            d = dir[from];

            if (d == abs(pawn_left) && (square_to_bit(to = from + pawn_left) & MoveGen::pawn_attacks(C, from) & enemyBB ))
            {
                if (Square::is_on_seventh_rank<C>(from))
                    push_capture_promotion(ml, from, to);
                else
                    add_capture_move(ml, from, to, PieceType::Pawn, cpiece[to], Move::FLAG_NONE);
            }
            else if (d == abs(pawn_right) && (square_to_bit(to = from + pawn_right) & MoveGen::pawn_attacks(C, from) & enemyBB))
            {
                if (Square::is_on_seventh_rank<C>(from))
                    push_capture_promotion(ml, from, to);
                else
                    add_capture_move(ml, from, to, PieceType::Pawn, cpiece[to], Move::FLAG_NONE);
            }

            if (/*do_quiet && */ d == abs(pawn_push) && (square_to_bit(to = from + pawn_push) & emptyBB))
            {
                add_quiet_move(ml, from, to, PieceType::Pawn, Move::FLAG_NONE);
                if (Square::is_on_second_rank<C>(from) && (square_to_bit(to += pawn_push) & emptyBB))
                    add_quiet_move(ml, from, to, PieceType::Pawn, Move::FLAG_DOUBLE);
            }
        }

        // bishop or queen (pinned)
        pieceBB = bq & pinnedBB;

        while (pieceBB)
        {
            from = next_square(pieceBB);
            d = dir[from];
            if (d == 9)
            {
                attackBB = MoveGen::bishop_moves(from, occupiedBB) & enemyBB & DiagonalMask64[from];
                push_capture_moves(ml, attackBB, from);
                attackBB = MoveGen::bishop_moves(from, occupiedBB) & emptyBB & DiagonalMask64[from];
                push_quiet_moves(ml, attackBB, from);
            }
            else if (d == 7)
            {
                attackBB = MoveGen::bishop_moves(from, occupiedBB) & enemyBB & AntiDiagonalMask64[from];
                push_capture_moves(ml, attackBB, from);
                attackBB = MoveGen::bishop_moves(from, occupiedBB) & emptyBB & AntiDiagonalMask64[from];
                push_quiet_moves(ml, attackBB, from);
            }
        }

        // rook or queen (pinned)
        pieceBB = rq & pinnedBB;

        while (pieceBB)
        {
            from = next_square(pieceBB);
            d = dir[from];
            if (d == 1)
            {
                attackBB = MoveGen::rook_moves(from, occupiedBB) & enemyBB & RankMask64[from];
                push_capture_moves(ml, attackBB, from);
                attackBB = MoveGen::rook_moves(from, occupiedBB) & emptyBB & RankMask64[from];
                push_quiet_moves(ml, attackBB, from);
            }
            else if (d == 8)
            {
                attackBB = MoveGen::rook_moves(from, occupiedBB) & enemyBB & FileMask64[from];
                push_capture_moves(ml, attackBB, from);
                attackBB = MoveGen::rook_moves(from, occupiedBB) & emptyBB & FileMask64[from];
                push_quiet_moves(ml, attackBB, from);
            }
        }
    }


    // common moves
    //    std::cout << "legal_gen common " << std::endl;

    // enpassant capture
    /*
     *      p P
     *
     *        P
     *
     * ep_square = e3 = 20
     * from      = d4 = 27
     * to
     *
     */

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

            if (!(MoveGen::bishop_moves(K, pieceBB) & bq & colorPiecesBB[O]) &&
                !(MoveGen::rook_moves(K, pieceBB)   & rq & colorPiecesBB[O]) )
            {
                add_capture_move(ml, from, to, PieceType::Pawn, PieceType::Pawn, Move::FLAG_ENPASSANT);
//                add_quiet_move(ml, from, to, PieceType::Pawn, Move::FLAG_ENPASSANT);
            }
        }

        from = ep + 1;          // 28 + 1 = 29 = f4
        if (Square::file(to) < 7 && our_pawns & square_to_bit(from) )
        {
            pieceBB = occupiedBB ^ square_to_bit(from) ^ square_to_bit(ep) ^ square_to_bit(to);

            if ( !(MoveGen::bishop_moves(K, pieceBB) & bq & colorPiecesBB[O]) &&
                !(MoveGen::rook_moves(K, pieceBB)   & rq & colorPiecesBB[O]) )
            {
                add_capture_move(ml, from, to, PieceType::Pawn, PieceType::Pawn, Move::FLAG_ENPASSANT);
//                add_quiet_move(ml, from, to, PieceType::Pawn, Move::FLAG_ENPASSANT);
            }
        }
    }


    // pawn
    pieceBB = typePiecesBB[Pawn] & unpinnedBB;

    attackBB = (C ? (pieceBB & ~FileMask8[0]) >> 9 : (pieceBB & ~FileMask8[0]) << 7) & enemyBB;
    push_capture_promotions(ml, attackBB & Promotion_Rank[C], pawn_left);
    push_pawn_capture_moves(ml, attackBB & ~Promotion_Rank[C], pawn_left);

    attackBB = (C ? (pieceBB & ~FileMask8[7]) >> 7 : (pieceBB & ~FileMask8[7]) << 9) & enemyBB;
    push_capture_promotions(ml, attackBB & Promotion_Rank[C], pawn_right);
    push_pawn_capture_moves(ml, attackBB & ~Promotion_Rank[C], pawn_right);

    attackBB = (C ? pieceBB >> 8 : pieceBB << 8) & emptyBB;
    push_quiet_promotions(ml, attackBB & Promotion_Rank[C], pawn_push);

    push_pawn_quiet_moves(ml, attackBB & ~Promotion_Rank[C], pawn_push, Move::FLAG_NONE);
    attackBB = (C ? (((pieceBB & RankMask8[6]) >> 8) & ~occupiedBB) >> 8 : (((pieceBB & RankMask8[1]) << 8) & ~occupiedBB) << 8) & emptyBB;
    push_pawn_quiet_moves(ml, attackBB, 2 * pawn_push, Move::FLAG_DOUBLE);

    // knight

    pieceBB = typePiecesBB[Knight] & unpinnedBB;
    while (pieceBB)
    {
        from = next_square(pieceBB);
        attackBB = MoveGen::knight_moves(from) & enemyBB;
        push_piece_capture_moves(ml, attackBB, from, PieceType::Knight);
        attackBB = MoveGen::knight_moves(from) & emptyBB;
        push_piece_quiet_moves(ml, attackBB, from, PieceType::Knight);
    }

    // bishop or queen
    pieceBB = bq & unpinnedBB;
    while (pieceBB)
    {
        from = next_square(pieceBB);
        attackBB = MoveGen::bishop_moves(from, occupiedBB) & enemyBB;
        push_capture_moves(ml, attackBB, from);
        attackBB = MoveGen::bishop_moves(from, occupiedBB) & emptyBB;
        push_quiet_moves(ml, attackBB, from);
    }

    // rook or queen
    pieceBB = rq & unpinnedBB;
    while (pieceBB)
    {
        from = next_square(pieceBB);
        attackBB = MoveGen::rook_moves(from, occupiedBB) & enemyBB;
        push_capture_moves(ml, attackBB, from);
        attackBB = MoveGen::rook_moves(from, occupiedBB) & emptyBB;
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

    auto mask = MoveGen::king_moves(K) & colorPiecesBB[O];
    while (mask)
    {
        to = next_square(mask);
        if (!square_attacked<O>(to))
            add_capture_move(ml, K, to, PieceType::King, cpiece[to], Move::FLAG_NONE);
    }
    mask = MoveGen::king_moves(K) & ~occupiedBB;
    while (mask)
    {
        to = next_square(mask);
        if (!square_attacked<O>(to))
            add_quiet_move(ml, K, to, PieceType::King, Move::FLAG_NONE);
    }

    // remet le roi dans l'échiquier
    colorPiecesBB[C] ^= square_to_bit(K);
}

// Explicit instantiations.
template void Board::legal_moves<WHITE>(MoveList& ml)  noexcept;
template void Board::legal_moves<BLACK>(MoveList& ml)  noexcept;

template void Board::apply_token<WHITE>(const std::string& token) noexcept;
template void Board::apply_token<BLACK>(const std::string& token) noexcept;
