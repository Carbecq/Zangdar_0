#include "defines.h"
#include "Board.h"
#include "evaluate.h"
#include "Square.h"

// Code de Vice (chap 82)
//  >> vient de Sjeng 11.2 (draw.c) et neval.c , ligne 588
//  >> qui vient de Faile
//  8/6R1/2k5/6P1/8/8/4nP2/6K1 w - - 1 41   : position vue dns la vidéo de Vice,
//  à qui sert-elle ?

//===========================================================================
//! \brief  Evaluation du camp 'Us' pour toutes les pièces
//! On ne prend en compte que le score matériel, et celui de la position des pièces
//!
//! \param[out] mg      score Middle game
//! \param[out] eg      score End game
//! \param[out] phase   phase de la partie
//---------------------------------------------------------------------------
template<Color Us>
constexpr void Board::fast_evaluate(Score& score, int &phase)
{
    Bitboard bb;

    // nullité
    // voir le code de Sjeng, qui comporte un test s'il reste des pions
    //  if (MaterialDraw() == true)
    //    return 0;

    score = 0;
    int sq;
    int sqpos;

    //-Pions---------------------------------------------------------------------
    bb = pieces_cp<Us, PieceType::Pawn>();
    while (bb)
    {
        score += meg_value[PieceType::Pawn];        // score matériel
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(Us, sq);              // case inversée pour les tables
        score += meg_pawn_table[sqpos];             // score positionnel
    }

    //-Cavaliers---------------------------------------------------------------------
    bb = pieces_cp<Us, PieceType::Knight>();
    while (bb)
    {
        phase += 1;

        score += meg_value[PieceType::Knight];
        sq     = next_square(bb);
        sqpos  = Square::flip(Us, sq);
        score += meg_knight_table[sqpos];
    }

    //-Fous---------------------------------------------------------------------
    bb = pieces_cp<Us, PieceType::Bishop>();
    while (bb)
    {
        phase += 1;

        score += meg_value[PieceType::Bishop];
        sq     = next_square(bb);
        sqpos  = Square::flip(Us, sq);
        score += meg_bishop_table[sqpos];
    }

    //-Tours---------------------------------------------------------------------
    bb = pieces_cp<Us, PieceType::Rook>();
    while (bb)
    {
        phase += 2;

        score += meg_value[PieceType::Rook];
        sq     = next_square(bb);
        sqpos  = Square::flip(Us, sq);
        score += meg_rook_table[sqpos];
    }

    //-Dames---------------------------------------------------------------------
    bb = pieces_cp<Us, PieceType::Queen>();
    while (bb)
    {
        phase += 4;

        score += meg_value[PieceType::Queen];
        sq     = next_square(bb);
        sqpos  = Square::flip(Us, sq);
        score += meg_queen_table[sqpos];
    }

    //-Roi---------------------------------------------------------------------

    score += meg_value[PieceType::King];
    score += meg_king_table[Square::flip(x_king[Us])];
}



template void Board::fast_evaluate<WHITE>(Score& score, int &phase);
template void Board::fast_evaluate<BLACK>(Score& score, int &phase);
