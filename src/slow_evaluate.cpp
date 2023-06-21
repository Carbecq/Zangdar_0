#include "defines.h"
#include "Board.h"
#include "evaluate.h"
#include "MoveGen.h"
#include "types.h"
#include "TranspositionTable.h"


extern Bitboard passed_pawn_mask[2][64];
extern Bitboard outpost_mask[2][64];
extern Bitboard rear_span_mask[2][64];
extern Bitboard backwards_mask[2][64];
extern Bitboard outer_kingring[64];

// Code de Vice (chap 82)
//  >> vient de Sjeng 11.2 (draw.c) et neval.c , ligne 588
//  >> qui vient de Faile

//===========================================================================
//! \brief  Evaluation pour toutes les pièces
//! \param[out] phase   phase de la partie
//! \return     score   score combiné (Middle game, End Game)
//---------------------------------------------------------------------------
Score Board::slow_evaluate(int& phase)
{
    Score score = 0;
    phase = 0;

    // nullité
    // voir le code de Sjeng, qui comporte un test s'il reste des pions
    //  if (MaterialDraw() == true)
    //    return 0;

    // Initialisation des bitboards
    constexpr Direction Down[2]    = {SOUTH, NORTH};                  // direction "bas", relativement à la couleur
    constexpr Bitboard  RelRanK2BB[2] = {                             // La rangée 2, relativement à la couleur
        RankMask8[Square::relative_rank<WHITE>(RANK_2)],
        RankMask8[Square::relative_rank<BLACK>(RANK_2)],
    };

    const Bitboard occupiedBB   = occupied();                                   // toutes les pièces (Blanches + Noires)
    const Bitboard UsPawnsBB[2] = {                                             // nos pions
                                    pieces_cp<WHITE, PieceType::Pawn>(),
                                    pieces_cp<BLACK, PieceType::Pawn>()
                                  };
    const Bitboard OtherPawnsBB[2] = {                                          // les pions de l'adversaire
                                        pieces_cp<BLACK, PieceType::Pawn>(),
                                        pieces_cp<WHITE, PieceType::Pawn>()
                                     };

    // La zone de mobilité est définie ainsi (idée de Weiss) : toute case
    //  > ni attaquée par un pion adverse
    //  > ni occupée par un pion ami sur sa case de départ
    //  > ni occupée par un pion ami bloqué

    const Bitboard b[2] = {
        (RelRanK2BB[WHITE] | shift<Down[WHITE]>(occupiedBB)) & UsPawnsBB[WHITE],
        (RelRanK2BB[BLACK] | shift<Down[BLACK]>(occupiedBB)) & UsPawnsBB[BLACK]
    };

    const Bitboard mobilityArea[2] ={
        ~(b[WHITE] | all_pawn_attacks<BLACK>(OtherPawnsBB[WHITE])),
        ~(b[BLACK] | all_pawn_attacks<WHITE>(OtherPawnsBB[BLACK]))
    };

#ifdef USE_CACHE
    Score pscore = 0;
    // Recherche de la position des pions dans le cache
    if (Transtable.probe_pawn_cache(pawn_hash, pscore) == true)
    {
        // La table de pions contient la position,
        // On récupère le score de la table
        score += pscore;
    }
    else
    {
        // La table de pions ne contient pas la position,
        // on calcule le score, et on le stocke
        pscore += evaluate_pawns<WHITE>(UsPawnsBB[WHITE], OtherPawnsBB[WHITE]);
        pscore -= evaluate_pawns<BLACK>(UsPawnsBB[BLACK], OtherPawnsBB[BLACK]);
        Transtable.store_pawn_cache(pawn_hash, pscore);
        score += pscore;
    }

#else
    score += evaluate_pawns<WHITE>(UsPawnsBB[WHITE], OtherPawnsBB[WHITE]);
    score -= evaluate_pawns<BLACK>(UsPawnsBB[BLACK], OtherPawnsBB[BLACK]);
#endif

    score += evaluate_knights<WHITE>(mobilityArea[WHITE], phase);
    score -= evaluate_knights<BLACK>(mobilityArea[BLACK], phase);

    score += evaluate_bishops<WHITE>(occupiedBB, mobilityArea[WHITE], phase);
    score -= evaluate_bishops<BLACK>(occupiedBB, mobilityArea[BLACK], phase);

    score += evaluate_rooks<WHITE>(UsPawnsBB[WHITE], OtherPawnsBB[WHITE], occupiedBB, mobilityArea[WHITE], phase);
    score -= evaluate_rooks<BLACK>(UsPawnsBB[BLACK], OtherPawnsBB[BLACK], occupiedBB, mobilityArea[BLACK], phase);

    score += evaluate_queens<WHITE>(UsPawnsBB[WHITE], occupiedBB, mobilityArea[WHITE], phase);
    score -= evaluate_queens<BLACK>(UsPawnsBB[BLACK], occupiedBB, mobilityArea[BLACK], phase);

    score += evaluate_king<WHITE>();
    score -= evaluate_king<BLACK>();

    return score;
}

//=================================================================
//! \brief  Evaluation des pions d'une couleur
//-----------------------------------------------------------------
template<Color Us>
Score Board::evaluate_pawns(const Bitboard UsPawnsBB, const Bitboard OtherPawnsBB)
{
    int sq, sqpos;
    Score score = 0;

    Bitboard bb = UsPawnsBB;

    // Pions doublés
    // des pions doublés désignent deux pions de la même couleur sur une même colonne.
#ifdef DEBUG_EVAL
            printf("Le camp %s possède %d pions doublés \n", side_name[Us].c_str(), Bcount(UsPawnsBB & north(UsPawnsBB)));
#endif
    score += PawnDoubled * Bcount(UsPawnsBB & north(UsPawnsBB));

    // Pions supportés par un autre pion
#ifdef DEBUG_EVAL
        printf("Le camp %s possède %d pions supportés par un autre pion \n", side_name[Us].c_str(), Bcount(UsPawnsBB & all_pawn_attacks<Us>(UsPawnsBB)));
#endif
    score += PawnSupport * Bcount(UsPawnsBB & all_pawn_attacks<Us>(UsPawnsBB));

    while (bb)
    {
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(Us, sq);              // case inversée pour les tables
        score += meg_value[PieceType::Pawn];        // score matériel
        score += meg_pawn_table[sqpos];             // score positionnel

#ifdef DEBUG_EVAL
            //            printf("le pion %s (%d) sur la case %s a une valeur MG de %d \n", side_name[Us].c_str(), Us, square_name[sq].c_str(), mg_pawn_table[sqpos]);
#endif
        // Pion isolé
        //  Un pion isolé est un pion qui n'a plus de pion de son camp sur les colonnes adjacentes.
        //  Un pion isolé peut être redoutable en milieu de partie1.
        //  C'est souvent une faiblesse en finale, car il est difficile à défendre.
        if ((UsPawnsBB & AdjacentFilesMask64[sq]) == 0)
        {
#ifdef DEBUG_EVAL
            printf("le pion %s sur la case %s est isolé \n", side_name[Us].c_str(), square_name[sq].c_str());
#endif
            score += PawnIsolated;
        }

        // Pion passé
        //  un pion passé est un pion qui n'est pas gêné dans son avance vers la 8e rangée par un pion adverse,
        //  c'est-à-dire qu'il n'y a pas de pion adverse devant lui, ni sur la même colonne, ni sur une colonne adjacente
        if ((passed_pawn_mask[Us][sq] & OtherPawnsBB) == 0)
        {
#ifdef DEBUG_EVAL
            printf("le pion %s sur la case %s est passé \n", side_name[Us].c_str(), square_name[sq].c_str());
#endif
            score += PawnPassed[Square::rank(sqpos)];
        }

        // Pion passé protégé

        // Pion passé connecté

        // tour derrière le pion passé

        // situation par rapport aux rois


        // Pion arriéré
        //  un pion arriéré est un pion qui est moins avancé que ceux des colonnes adjacentes
        //  et ne peut plus bénéficier de leur protection1.
            //        if ((backwards_mask[Us][sq] & UsPawnsBB) == 0)
            //        {
            //            if (debug)
            //                printf("le pion %s sur la case %s est arriéré \n", side_name[Us].c_str(), square_name[sq].c_str());
            //            score += PawnBackward;
            //        }
    }

    return score;
}

//=================================================================
//! \brief  Evaluation des cavaliers d'une couleur
//-----------------------------------------------------------------
template<Color Us>
Score Board::evaluate_knights(const Bitboard mobilityArea, int &phase)
{
    int sq, sqpos;
    int count;
    Score score = 0;
    Bitboard bb = pieces_cp<Us, PieceType::Knight>();
    while (bb)
    {
        phase += 1;
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(Us, sq);              // case inversée pour les tables
        score += meg_value[PieceType::Knight];      // score matériel
        score += meg_knight_table[sqpos];           // score positionnel

        // mobilité
        count = Bcount(MoveGen::knight_moves(sq) & mobilityArea);
        score += KnightMobility[count];
    }
    return score;
}

//=================================================================
//! \brief  Evaluation des fous d'une couleur
//-----------------------------------------------------------------
template<Color Us>
Score Board::evaluate_bishops(const Bitboard occupiedBB, const Bitboard mobilityArea, int &phase)
{
    int sq, sqpos;
    int count;
    Score score = 0;
    Bitboard bb = pieces_cp<Us, PieceType::Bishop>();
    if (Bcount(bb) >= 2)
    {
        // Paire de fous
        // On ne teste pas si les fous sont de couleur différente !
#ifdef DEBUG_EVAL
            printf("Le camp %s possède la paire de fous \n", side_name[Us].c_str());
#endif
        score += BishopPair;
    }

    while (bb)
    {
        phase += 1;
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(Us, sq);              // case inversée pour les tables
        score += meg_value[PieceType::Bishop];      // score matériel
        score += meg_bishop_table[sqpos];           // score positionnel

        // mobilité
        count = Bcount(MoveGen::bishop_moves(sq, occupiedBB) & mobilityArea);
        score += BishopMobility[count];
    }
    return score;
}

//=================================================================
//! \brief  Evaluation des tours d'une couleur
//-----------------------------------------------------------------
template<Color Us>
Score Board::evaluate_rooks(const Bitboard UsPawnsBB, const Bitboard OtherPawnsBB,
                            const Bitboard occupiedBB, const Bitboard mobilityArea, int &phase)
{
    int sq, sqpos;
    int count;
    Score score = 0;

    Bitboard bb = pieces_cp<Us, PieceType::Rook>();
    while (bb)
    {
        phase += 2;
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(Us, sq);              // case inversée pour les tables
        score += meg_value[PieceType::Rook];    // score matériel
        score += meg_rook_table[sqpos];            // score positionnel

        // mobilité
        count = Bcount(MoveGen::rook_moves(sq, occupiedBB) & mobilityArea);
        score += RookMobility[count];

        // semi-open file
        if (((UsPawnsBB | OtherPawnsBB) & FileMask64[sq]) == 0) // pas de pion ami ou ennemi
        {
#ifdef DEBUG_EVAL
                printf("la tour %s sur la case %s est sur une colonne ouverte \n",
                       side_name[Us].c_str(), square_name[sq].c_str());
#endif
            score += RookOpenFile;
        }

        // open file
        else if ((UsPawnsBB & FileMask64[sq]) == 0) // pas de pion ami
        {
#ifdef DEBUG_EVAL
                printf("la tour %s sur la case %s est sur une colonne semi-ouverte \n",
                       side_name[Us].c_str(), square_name[sq].c_str());
#endif
            score += RookSemiOpenFile;
        }

    }
    return score;
}

//=================================================================
//! \brief  Evaluation des reines d'une couleur
//-----------------------------------------------------------------
template<Color Us>
Score Board::evaluate_queens(const Bitboard UsPawnsBB, const Bitboard occupiedBB, const Bitboard mobilityArea, int &phase)
{
    int sq, sqpos;
    int count;
    Score score = 0;
    Bitboard bb = pieces_cp<Us, PieceType::Queen>();
    while (bb)
    {
        phase += 4;
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(Us, sq);              // case inversée pour les tables
        score += meg_value[PieceType::Queen];    // score matériel
        score += meg_queen_table[sqpos];            // score positionnel

        // mobilité
        count = Bcount(MoveGen::queen_moves(sq, occupiedBB) & mobilityArea);
        score += QueenMobility[count];

        // Colonnes ouvertes
        if ((typePiecesBB[PieceType::Pawn] & FileMask64[sq]) == 0) // pas de pion ami ou ennemi
            score += QueenOpenFile;
        else if ((UsPawnsBB & FileMask64[sq]) == 0) // pas de pion ami
            score += QueenSemiOpenFile;
    }
    return score;
}

//=================================================================
//! \brief  Evaluation du roi d'une couleur
//-----------------------------------------------------------------
template<Color Us>
Score Board::evaluate_king()
{
    int sq, sqpos;
    Score score = 0;
    sq     = x_king[Us];
    sqpos  = Square::flip(Us, sq);              // case inversée pour les tables
    score += meg_value[PieceType::King];        // score matériel
    score += meg_king_table[sqpos];             // score positionnel

    // sécurité du roi
    // On compte le nombre de case d'où on peut attaquer le roi (sauf cavalier)
    score += KingLineDanger * Bcount(MoveGen::queen_moves(sq, colorPiecesBB[Us] | typePiecesBB[PieceType::Pawn]));

    return score;
}

template Score Board::evaluate_pawns<WHITE>(const Bitboard UsPawnsBB, const Bitboard OtherPawnsBB);
template Score Board::evaluate_pawns<BLACK>(const Bitboard UsPawnsBB, const Bitboard OtherPawnsBB);

template Score Board::evaluate_knights<WHITE>(const Bitboard mobilityArea, int &phase);
template Score Board::evaluate_knights<BLACK>(const Bitboard mobilityArea, int &phase);

template Score Board::evaluate_bishops<WHITE>(const Bitboard occupiedBB, const Bitboard mobilityArea, int &phase);
template Score Board::evaluate_bishops<BLACK>(const Bitboard occupiedBB, const Bitboard mobilityArea, int &phase);

template Score Board::evaluate_rooks<WHITE>(const Bitboard UsPawnsBB, const Bitboard OtherPawnsBB, const Bitboard occupiedBB, const Bitboard mobilityArea, int &phase);
template Score Board::evaluate_rooks<BLACK>(const Bitboard UsPawnsBB, const Bitboard OtherPawnsBB, const Bitboard occupiedBB, const Bitboard mobilityArea, int &phase);

template Score Board::evaluate_queens<WHITE>(const Bitboard UsPawnsBB, const Bitboard occupiedBB, const Bitboard mobilityArea, int &phase);
template Score Board::evaluate_queens<BLACK>(const Bitboard UsPawnsBB, const Bitboard occupiedBB, const Bitboard mobilityArea, int &phase);

template Score Board::evaluate_king<WHITE>();
template Score Board::evaluate_king<BLACK>();
