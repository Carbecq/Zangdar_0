#include <iostream>
#include "Board.h"
#include "defines.h"
#include "evaluation.h"


// Voir Sjeng : neval.c

//==========================================
//! \brief  Evaluation de la position
//------------------------------------------
[[nodiscard]] int Board::evaluate()
{
    int mg[2];
    int eg[2];
    int gamePhase = 0;

    evaluate_0<WHITE>(mg[WHITE], eg[WHITE], gamePhase);
    evaluate_0<BLACK>(mg[BLACK], eg[BLACK], gamePhase);


    int mgScore = mg[WHITE] - mg[BLACK];
    int egScore = eg[WHITE] - eg[BLACK];
    int mgPhase = gamePhase;


    if (mgPhase > 24)
        mgPhase = 24; // in case of early promotion
    int egPhase = 24 - mgPhase;


    I32 score = (mgScore * mgPhase + egScore * egPhase) / 24;


    // return score relative to the side to move
    if (side_to_move == WHITE)
        return score;
    else
        return -score;
}

template <Color C>
constexpr void Board::evaluate_0(int& mg, int& eg, int& gamePhase)
{
    int sq, sqpos;


    mg = 0;
    eg = 0;

    {
        constexpr PieceType pt = PieceType::Pawn;
        Bitboard  bb = pieces_cp<C, PieceType::Pawn>();
        while (bb)
        {
            // score matériel
            mg += mg_value[pt];
            eg += eg_value[pt];

            // case où est la pièce
            sq = next_square(bb);

            // score positionnel
            // il faut intervertir les cases pour les Noirs
            sqpos = Square::flip(C, sq);
#ifdef DEBUG_EVAL
 //               printf("le pion %s (%d) sur la case %s a une valeur MG de %d \n", side_name[C].c_str(), C, square_name[sq].c_str(), PSQT_MG[pt][sqpos]);
#endif

            mg += mg_pawn_table[sqpos];
            eg += eg_pawn_table[sqpos];

        }
    }

    {
        constexpr PieceType pt  = PieceType::Knight;
        Bitboard bb = pieces_cp<C, PieceType::Knight>();
        while (bb)
        {
            gamePhase += 1;

            // score matériel
            mg += mg_value[pt];
            eg += eg_value[pt];

            // case où est la pièce
            sq = next_square(bb);

            // score positionnel
            // il faut intervertir les cases
            // car les tables sont données à l'envers
            sqpos = Square::flip(C, sq);

            mg += mg_knight_table[sqpos];
            eg += eg_knight_table[sqpos];

        }
    }

    {
        constexpr PieceType pt   = PieceType::Bishop;
        Bitboard bb = pieces_cp<C, PieceType::Bishop>();
        while (bb)
        {
            gamePhase += 1;

            // score matériel
            mg += mg_value[pt];
            eg += eg_value[pt];

            // case où est la pièce
            sq = next_square(bb);

            // score positionnel
            // il faut intervertir les cases
            sqpos = Square::flip(C, sq);

            mg += mg_bishop_table[sqpos];
            eg += eg_bishop_table[sqpos];

        }
    }

    {
        constexpr PieceType pt  = PieceType::Rook;
        Bitboard bb = pieces_cp<C, PieceType::Rook>();
        while (bb)
        {
             gamePhase += 2;

            // score matériel
            mg += mg_value[pt];
            eg += eg_value[pt];

            // case où est la pièce
            sq = next_square(bb);

            // score positionnel
            // il faut intervertir les cases
            // car les tables sont données à l'envers
            sqpos = Square::flip(C, sq);

            mg += mg_rook_table[sqpos];
            eg += eg_rook_table[sqpos];

        }
    }

    {
        constexpr PieceType pt   = PieceType::Queen;
        Bitboard bb = pieces_cp<C, PieceType::Queen>();
        while (bb)
        {
             gamePhase += 4;

            // score matériel
            mg += mg_value[pt];
            eg += eg_value[pt];

            // case où est la pièce
            sq = next_square(bb);

            // score positionnel
            // il faut intervertir les cases
            sqpos = Square::flip(C, sq);

            mg += mg_queen_table[sqpos];
            eg += eg_queen_table[sqpos];

        }
    }

    {
        constexpr PieceType pt   = PieceType::King;
        Bitboard bb = pieces_cp<C, PieceType::King>();
        while (bb)
        {
            // score matériel
            mg += mg_value[pt];
            eg += eg_value[pt];

             // case où est la pièce
            sq = next_square(bb);

            // score positionnel
            // il faut intervertir les cases
            sqpos = Square::flip(C, sq);

            mg += mg_king_table[sqpos];
            eg += eg_king_table[sqpos];

        }
    }

}


template void Board::evaluate_0<WHITE>(int& mg, int& eg, int& gamePhase);
template void Board::evaluate_0<BLACK>(int& mg, int& eg, int& gamePhase);
