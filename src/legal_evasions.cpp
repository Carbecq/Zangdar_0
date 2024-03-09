#include "Board.h"
#include "Bitboard.h"
#include "Attacks.h"
#include "Square.h"
#include "Move.h"

constexpr int PUSH[] = {8, -8};


//=================================================================
//! \brief  Génération de tous les coups légaux, échappant à un échec
//! \param  ml  Liste des coups dans laquelle on va stocker les coups
//!
//! algorithme de Mperft
//-----------------------------------------------------------------
template <Color C>
constexpr void Board::legal_evasions(MoveList& ml) noexcept
{
    constexpr Color Them = ~C;
    const int   K = x_king[C];
    
    const Bitboard occupiedBB = occupancy_all();     // toutes les pièces (Blanches + Noires)
    Bitboard emptyBB    = ~occupiedBB;
    const Bitboard usBB       = colorPiecesBB[C];
    Bitboard enemyBB    = colorPiecesBB[Them];

    const Bitboard bq         = typePiecesBB[BISHOP] | typePiecesBB[QUEEN];
    const Bitboard rq         = typePiecesBB[ROOK]   | typePiecesBB[QUEEN];

    //-----------------------------------------------------------------------------------------
    //  Calcul des pièces clouées, et des échecs
    //  algorithme de Surge
    //-----------------------------------------------------------------------------------------

    int s;
    Bitboard b1;

    //    const Bitboard our_diag_sliders   = diagonal_sliders<C>();
    const Bitboard their_diag_sliders = diagonal_sliders<Them>();
    //    const Bitboard our_orth_sliders   = orthogonal_sliders<C>();
    const Bitboard their_orth_sliders = orthogonal_sliders<Them>();

    //Checkers of each piece type are identified by:
    //1. Projecting attacks FROM the king square
    //2. Intersecting this bitboard with the enemy bitboard of that piece type
    Bitboard checkersBB = (Attacks::knight_moves(K)    & occupancy_cp<Them, KNIGHT>() ) |
                          (Attacks::pawn_attacks<C>(K) & occupancy_cp<Them, PAWN>() );

    //Here, we identify slider checkers and pinners simultaneously, and candidates for such pinners
    //and checkers are represented by the bitboard <candidates>
    Bitboard candidates = (Attacks::rook_moves(K, enemyBB)   & their_orth_sliders) |
                          (Attacks::bishop_moves(K, enemyBB) & their_diag_sliders);

    Bitboard pinnedBB = 0;
    while (candidates)
    {
        s  = BB::pop_lsb(candidates);
        b1 = squares_between(K, s) & usBB;

        //Do the squares in between the enemy slider and our king contain any of our pieces?
        //If not, add the slider to the checker bitboard
        if (b1 == 0)
            checkersBB |= BB::sq2BB(s);

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

    //    std::cout << "legal_gen 1 ; ch=%d " << BB::count_bit(checkersBB) << std::endl;

    // in check: capture or block the (single) checker if any;
    if (checkersBB)
    {
        if (BB::count_bit(checkersBB) == 1)
        {
            x_checker = BB::get_lsb(checkersBB);
            emptyBB = squares_between(K, x_checker);
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
        
        Bitboard our_pawns = occupancy_cp<C, PAWN>();
        if (SQ::file(to) > 0 && our_pawns & BB::sq2BB(from) )
        {
            pieceBB = occupiedBB ^ BB::sq2BB(from) ^ BB::sq2BB(ep) ^ BB::sq2BB(to);

            if ( !(Attacks::bishop_moves(K, pieceBB) & bq & colorPiecesBB[Them]) &&
                !(Attacks::rook_moves(K, pieceBB)   & rq & colorPiecesBB[Them]) )
            {
               add_capture_move(ml, from, to, PAWN, PAWN, Move::FLAG_ENPASSANT);
            }
        }

        from = ep + 1;          // 28 + 1 = 29 = f4
        if (SQ::file(to) < 7 && our_pawns & BB::sq2BB(from) )
        {
            pieceBB = occupiedBB ^ BB::sq2BB(from) ^ BB::sq2BB(ep) ^ BB::sq2BB(to);

            if ( !(Attacks::bishop_moves(K, pieceBB) & bq & colorPiecesBB[Them]) &&
                !(Attacks::rook_moves(K, pieceBB)   & rq & colorPiecesBB[Them]) )
            {
                add_capture_move(ml, from, to, PAWN, PAWN, Move::FLAG_ENPASSANT);
            }
        }
    }


    // pawn
    pieceBB = typePiecesBB[PAWN] & unpinnedBB;

    attackBB = (C ? (pieceBB & ~FileMask8[0]) >> 9 : (pieceBB & ~FileMask8[0]) << 7) & enemyBB;
    push_capture_promotions(ml, attackBB & PromotionRank[C], pawn_left);
    push_pawn_capture_moves(ml, attackBB & ~PromotionRank[C], pawn_left);

    attackBB = (C ? (pieceBB & ~FileMask8[7]) >> 7 : (pieceBB & ~FileMask8[7]) << 9) & enemyBB;
    push_capture_promotions(ml, attackBB & PromotionRank[C], pawn_right);
    push_pawn_capture_moves(ml, attackBB & ~PromotionRank[C], pawn_right);

    attackBB = (C ? pieceBB >> 8 : pieceBB << 8) & emptyBB;
    push_quiet_promotions(ml, attackBB & PromotionRank[C], pawn_push);

    push_pawn_quiet_moves(ml, attackBB & ~PromotionRank[C], pawn_push, Move::FLAG_NONE);
    attackBB = (C ? (((pieceBB & RankMask8[6]) >> 8) & ~occupiedBB) >> 8 : (((pieceBB & RankMask8[1]) << 8) & ~occupiedBB) << 8) & emptyBB;
    push_pawn_quiet_moves(ml, attackBB, 2 * pawn_push, Move::FLAG_DOUBLE);

    // knight

    pieceBB = typePiecesBB[KNIGHT] & unpinnedBB;
    while (pieceBB)
    {
        from = BB::pop_lsb(pieceBB);
        attackBB = Attacks::knight_moves(from) & enemyBB;
        push_piece_capture_moves(ml, attackBB, from, KNIGHT);
        attackBB = Attacks::knight_moves(from) & emptyBB;
        push_piece_quiet_moves(ml, attackBB, from, KNIGHT);
    }

    // bishop or queen
    pieceBB = bq & unpinnedBB;
    while (pieceBB)
    {
        from = BB::pop_lsb(pieceBB);
        attackBB = Attacks::bishop_moves(from, occupiedBB) & enemyBB;
        push_capture_moves(ml, attackBB, from);
        attackBB = Attacks::bishop_moves(from, occupiedBB) & emptyBB;
        push_quiet_moves(ml, attackBB, from);
    }

    // rook or queen
    pieceBB = rq & unpinnedBB;
    while (pieceBB)
    {
        from = BB::pop_lsb(pieceBB);
        attackBB = Attacks::rook_moves(from, occupiedBB) & enemyBB;
        push_capture_moves(ml, attackBB, from);
        attackBB = Attacks::rook_moves(from, occupiedBB) & emptyBB;
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
    colorPiecesBB[C] ^= BB::sq2BB(K);

    auto mask = Attacks::king_moves(K) & colorPiecesBB[Them];
    while (mask)
    {
        to = BB::pop_lsb(mask);
        if (!square_attacked<Them>(to))
            add_capture_move(ml, K, to, KING, pieceOn[to], Move::FLAG_NONE);
    }
    mask = Attacks::king_moves(K) & ~occupiedBB;
    while (mask)
    {
        to = BB::pop_lsb(mask);
        if (!square_attacked<Them>(to))
            add_quiet_move(ml, K, to, KING, Move::FLAG_NONE);
    }

    // remet le roi dans l'échiquier
    colorPiecesBB[C] ^= BB::sq2BB(K);
}

// Explicit instantiations.
template void Board::legal_evasions<WHITE>(MoveList& ml) noexcept;
template void Board::legal_evasions<BLACK>(MoveList& ml) noexcept;
