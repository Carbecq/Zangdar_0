#include "defines.h"
#include "Board.h"
#include "evaluate.h"
#include "MoveGen.h"
#include "types.h"
#include "TranspositionTable.h"
#include "Square.h"

extern Bitboard passed_pawn_mask[2][64];
extern Bitboard outpost_mask[2][64];
extern Bitboard rear_span_mask[2][64];
extern Bitboard backwards_mask[2][64];
extern Bitboard outer_kingring[64];


//===========================================================================
//! \brief  Evaluation pour toutes les pièces
//! \param[out] phase   phase de la partie
//! \return     score   score combiné (Middle game, End Game)
//---------------------------------------------------------------------------
Score Board::slow_evaluate(int& phase)
{
    Score score = 0;
    EvalInfo ei;

    // nullité
    if (material_draw() == true)
        return 0;


    // Initialisation des bitboards
    constexpr Direction Down[2] = {SOUTH, NORTH};                  // direction "bas", relativement à la couleur
    constexpr Direction Up[2]   = {NORTH, SOUTH};                  // direction "haut", relativement à la couleur
    constexpr Bitboard  RelRanK2BB[2] = {                             // La rangée 2, relativement à la couleur
        RankMask8[Square::relative_rank<WHITE>(RANK_2)],
        RankMask8[Square::relative_rank<BLACK>(RANK_2)],
    };

    ei.phase = 0;
    ei.attackCount[WHITE] = 0;
    ei.attackCount[BLACK] = 0;
    ei.attackPower[WHITE] = 0;
    ei.attackPower[BLACK] = 0;

    ei.occupiedBB   = occupied();                                   // toutes les pièces (Blanches + Noires)
    ei.pawns[WHITE] = pieces_cp<WHITE, PieceType::Pawn>();
    ei.pawns[BLACK] = pieces_cp<BLACK, PieceType::Pawn>();

    ei.knights[WHITE] = pieces_cp<WHITE, PieceType::Knight>();
    ei.knights[BLACK] = pieces_cp<BLACK, PieceType::Knight>();

    ei.bishops[WHITE] = pieces_cp<WHITE, PieceType::Bishop>();
    ei.bishops[BLACK] = pieces_cp<BLACK, PieceType::Bishop>();

    ei.rooks[WHITE] = pieces_cp<WHITE, PieceType::Rook>();
    ei.rooks[BLACK] = pieces_cp<BLACK, PieceType::Rook>();

    ei.queens[WHITE] = pieces_cp<WHITE, PieceType::Queen>();
    ei.queens[BLACK] = pieces_cp<BLACK, PieceType::Queen>();

    // La zone de mobilité est définie ainsi (idée de Weiss) : toute case
    //  > ni attaquée par un pion adverse
    //  > ni occupée par un pion ami sur sa case de départ
    //  > ni occupée par un pion ami bloqué

    const Bitboard b[2] = {
        (RelRanK2BB[WHITE] | shift<Down[WHITE]>(ei.occupiedBB)) & ei.pawns[WHITE],
        (RelRanK2BB[BLACK] | shift<Down[BLACK]>(ei.occupiedBB)) & ei.pawns[BLACK]
    };

    ei.mobilityArea[WHITE] = ~(b[WHITE] | all_pawn_attacks<BLACK>(ei.pawns[BLACK]));
    ei.mobilityArea[BLACK] = ~(b[BLACK] | all_pawn_attacks<WHITE>(ei.pawns[WHITE]));

    // Sécurité du roi
    const Bitboard auxi[2] = {
        MoveGen::king_moves(x_king[BLACK]),
        MoveGen::king_moves(x_king[WHITE])
    };

    // Bitboard des cases entourant le roi ennemi
    // et d'une rangée de cases vers le Nord (pour les blancs)
    ei.enemyKingZone[WHITE] = auxi[WHITE] | shift<Up[WHITE]>(auxi[WHITE]);
    ei.enemyKingZone[BLACK] = auxi[BLACK] | shift<Up[BLACK]>(auxi[BLACK]);

    // Attaques des pions
    ei.pawnAttacks[WHITE] = all_pawn_attacks<WHITE>(ei.pawns[WHITE]);
    ei.pawnAttacks[BLACK] = all_pawn_attacks<BLACK>(ei.pawns[BLACK]);

    //--------------------------------
    //  Evaluation des pions
    //--------------------------------

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
        pscore += evaluate_pawns<WHITE>(ei);
        pscore -= evaluate_pawns<BLACK>(ei);
        Transtable.store_pawn_cache(pawn_hash, pscore);
        score += pscore;
    }
#else
    score += evaluate_pawns<WHITE>(UsPawnsBB[WHITE], OtherPawnsBB[WHITE]);
    score -= evaluate_pawns<BLACK>(UsPawnsBB[BLACK], OtherPawnsBB[BLACK]);
#endif

    //--------------------------------
    // Evaluation des pieces
    //--------------------------------

    score += evaluate_knights<WHITE>(ei);
    score -= evaluate_knights<BLACK>(ei);

    score += evaluate_bishops<WHITE>(ei);
    score -= evaluate_bishops<BLACK>(ei);

    score += evaluate_rooks<WHITE>(ei);
    score -= evaluate_rooks<BLACK>(ei);

    score += evaluate_queens<WHITE>(ei);
    score -= evaluate_queens<BLACK>(ei);

    score += evaluate_king<WHITE>(ei);
    score -= evaluate_king<BLACK>(ei);

    //--------------------------------
    // Evaluation de la sécurité du roi
    //--------------------------------

    score += evaluate_safety<WHITE>(ei);
    score -= evaluate_safety<BLACK>(ei);

    phase = ei.phase;
    return score;
}

//=================================================================
//! \brief  Evaluation des pions d'une couleur
//-----------------------------------------------------------------
template<Color C>
Score Board::evaluate_pawns(EvalInfo& ei)
{
    int sq, sqpos;
    Score score = 0;

    Bitboard pawns = ei.pawns[C];

    // Pions doublés
    // des pions doublés désignent deux pions de la même couleur sur une même colonne.
#ifdef DEBUG_EVAL
            printf("Le camp %s possède %d pions doublés \n", side_name[Us].c_str(), Bcount(UsPawnsBB & north(UsPawnsBB)));
#endif
    score += PawnDoubled * Bcount(pawns & north(pawns));

    // Pions supportés par un autre pion
#ifdef DEBUG_EVAL
        printf("Le camp %s possède %d pions supportés par un autre pion \n", side_name[Us].c_str(), Bcount(UsPawnsBB & all_pawn_attacks<Us>(UsPawnsBB)));
#endif
    score += PawnSupport * Bcount(pawns & all_pawn_attacks<C>(pawns));

    Bitboard bb = ei.pawns[C];
    while (bb)
    {
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(C, sq);              // case inversée pour les tables
        score += meg_value[PieceType::Pawn];        // score matériel
        score += meg_pawn_table[sqpos];             // score positionnel

#ifdef DEBUG_EVAL
            //            printf("le pion %s (%d) sur la case %s a une valeur MG de %d \n", side_name[Us].c_str(), Us, square_name[sq].c_str(), mg_pawn_table[sqpos]);
#endif
        // Pion isolé
        //  Un pion isolé est un pion qui n'a plus de pion de son camp sur les colonnes adjacentes.
        //  Un pion isolé peut être redoutable en milieu de partie1.
        //  C'est souvent une faiblesse en finale, car il est difficile à défendre.
        if ((pawns & AdjacentFilesMask64[sq]) == 0)
        {
#ifdef DEBUG_EVAL
            printf("le pion %s sur la case %s est isolé \n", side_name[Us].c_str(), square_name[sq].c_str());
#endif
            score += PawnIsolated;
        }

        // Pion passé
        //  un pion passé est un pion qui n'est pas gêné dans son avance vers la 8e rangée par un pion adverse,
        //  c'est-à-dire qu'il n'y a pas de pion adverse devant lui, ni sur la même colonne, ni sur une colonne adjacente
        if ((passed_pawn_mask[C][sq] & ei.pawns[~C]) == 0)
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
template<Color C>
Score Board::evaluate_knights(EvalInfo& ei)
{
    int sq, sqpos;
    int count;
    Score score = 0;

    Bitboard bb = ei.knights[C];
    ei.phase += Bcount(bb);

    while (bb)
    {
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(C, sq);              // case inversée pour les tables
        score += meg_value[PieceType::Knight];      // score matériel
        score += meg_knight_table[sqpos];           // score positionnel

        // mobilité
        Bitboard mobilityBB = MoveGen::knight_moves(sq) & ei.mobilityArea[C];
        count = Bcount(mobilityBB);
        score += KnightMobility[count];

        //  attaques et échecs pour calculer la sécurité du roi
        int nbr_attacks = Bcount(mobilityBB & ei.enemyKingZone[C]);
        int nbr_checks  = Bcount(mobilityBB & MoveGen::knight_moves(x_king[~C]));

        if (nbr_attacks > 0 || nbr_checks > 0)
        {
            ei.attackCount[C]++;
            ei.attackPower[C] += nbr_attacks * AttackPower[PieceType::Knight] + nbr_checks * CheckPower[PieceType::Knight];
        }
    }
    return score;
}

//=================================================================
//! \brief  Evaluation des fous d'une couleur
//-----------------------------------------------------------------
template<Color C>
Score Board::evaluate_bishops(EvalInfo& ei)
{
    int sq, sqpos;
    int count;
    Score score = 0;
    Bitboard bb = ei.bishops[C];
    ei.phase += Bcount(bb);

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
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(C, sq);              // case inversée pour les tables
        score += meg_value[PieceType::Bishop];      // score matériel
        score += meg_bishop_table[sqpos];           // score positionnel

        // mobilité
        Bitboard mobilityBB = XRayBishopAttack<C>(sq) & ei.mobilityArea[C];
        count = Bcount(mobilityBB);
        score += BishopMobility[count];

        //  attaques et échecs pour calculer la sécurité du roi
        int nbr_attacks = Bcount(mobilityBB & ei.enemyKingZone[C]);
        int nbr_checks  = Bcount(mobilityBB & MoveGen::bishop_moves(x_king[~C], ei.occupiedBB));

        if (nbr_attacks > 0 || nbr_checks > 0)
        {
            ei.attackCount[C] ++;
            ei.attackPower[C] += nbr_attacks * AttackPower[PieceType::Bishop] + nbr_checks * CheckPower[PieceType::Bishop];
        }
    }
    return score;
}

//=================================================================
//! \brief  Evaluation des tours d'une couleur
//-----------------------------------------------------------------
template<Color C>
Score Board::evaluate_rooks(EvalInfo& ei)
{
    int sq, sqpos;
    int count;
    Score score = 0;

    Bitboard bb = ei.rooks[C];
    ei.phase += 2*Bcount(bb);

    while (bb)
    {
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(C, sq);              // case inversée pour les tables
        score += meg_value[PieceType::Rook];    // score matériel
        score += meg_rook_table[sqpos];            // score positionnel

        // mobilité
        Bitboard mobilityBB = XRayRookAttack<C>(sq) & ei.mobilityArea[C];
        count = Bcount(mobilityBB);
        score += RookMobility[count];

        //  attaques et échecs pour calculer la sécurité du roi
        int nbr_attacks = Bcount(mobilityBB & ei.enemyKingZone[C]);
        int nbr_checks  = Bcount(mobilityBB & MoveGen::rook_moves(x_king[~C], ei.occupiedBB));

        if (nbr_attacks > 0 || nbr_checks > 0)
        {
            ei.attackCount[C]++;
            ei.attackPower[C] += nbr_attacks * AttackPower[PieceType::Rook] + nbr_checks * CheckPower[PieceType::Rook];
        }

        // Colonnes ouvertes
        if (((ei.pawns[WHITE] | ei.pawns[BLACK]) & FileMask64[sq]) == 0) // pas de pion ami ou ennemi
        {
#ifdef DEBUG_EVAL
                printf("la tour %s sur la case %s est sur une colonne ouverte \n",
                       side_name[Us].c_str(), square_name[sq].c_str());
#endif
            score += RookOpenFile;
        }

        // Colonnes semi-ouvertes
        else if ((ei.pawns[C] & FileMask64[sq]) == 0) // pas de pion ami
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
template<Color C>
Score Board::evaluate_queens(EvalInfo& ei)
{
    int sq, sqpos;
    int count;
    Score score = 0;
    Bitboard bb = ei.queens[C];
    ei.phase += 4*Bcount(bb);

    while (bb)
    {
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(C, sq);              // case inversée pour les tables
        score += meg_value[PieceType::Queen];    // score matériel
        score += meg_queen_table[sqpos];            // score positionnel

        // mobilité
        Bitboard mobilityBB = XRayQueenAttack<C>(sq) & ei.mobilityArea[C];
        count = Bcount(mobilityBB);
        score += QueenMobility[count];

        //  attaques et échecs pour calculer la sécurité du roi

        // nbr_attacks : nombre d'attaques sur la zone du roi ennemi
        //          à noter que seules les cases de la mobilité comptent
        int nbr_attacks = Bcount(mobilityBB & ei.enemyKingZone[C]);

        // nbr_checks = nombre de cases d'où la dame peut faire échec
        //          à noter que seules les cases de la mobilité comptent
        int nbr_checks     = Bcount(mobilityBB & MoveGen::queen_moves(x_king[~C], ei.occupiedBB));

        if (nbr_attacks > 0 || nbr_checks > 0)
        {
            ei.attackCount[C]++;
            ei.attackPower[C] += nbr_attacks * AttackPower[PieceType::Queen]
                               + nbr_checks  * CheckPower[PieceType::Queen];
        }

        // Colonnes ouvertes
        if (((ei.pawns[WHITE] | ei.pawns[BLACK]) & FileMask64[sq]) == 0) // pas de pion ami ou ennemi
        {
#ifdef DEBUG_EVAL
            printf("la Dame %s sur la case %s est sur une colonne ouverte \n",
                   side_name[Us].c_str(), square_name[sq].c_str());
#endif
            score += QueenOpenFile;
        }

        // Colonnes semi-ouvertes
        else if ((ei.pawns[C] & FileMask64[sq]) == 0) // pas de pion ami
        {
#ifdef DEBUG_EVAL
            printf("la dame %s sur la case %s est sur une colonne semi-ouverte \n",
                   side_name[Us].c_str(), square_name[sq].c_str());
#endif
            score += QueenSemiOpenFile;
        }
    }
    return score;
}

//=================================================================
//! \brief  Evaluation du roi d'une couleur
//-----------------------------------------------------------------
template<Color C>
Score Board::evaluate_king(EvalInfo& ei)
{
    int sq, sqpos;
    Score score = 0;
    sq     = x_king[C];
    sqpos  = Square::flip(C, sq);              // case inversée pour les tables
    score += meg_value[PieceType::King];       // score matériel
    score += meg_king_table[sqpos];            // score positionnel

    // sécurité du roi
    // On compte le nombre de case d'où on peut attaquer le roi (sauf cavalier)
    Bitboard SafeLine = RankMask8[Square::relative_rank<C>(RANK_1)];
    int count = Bcount(~SafeLine & MoveGen::queen_moves(sq, colorPiecesBB[C] | typePiecesBB[PieceType::Pawn]));
    score += KingLineDanger[count];

    // Add to enemy's attack power based on open lines
    ei.attackPower[~C] += (count - 3) * 8;

    return score;
}

//=================================================================
//! \brief  Evaluation de la sécurité du roi
//-----------------------------------------------------------------
template<Color C>
Score Board::evaluate_safety(const EvalInfo& ei)
{
    int score = ei.attackPower[C] * CountModifier[std::min(7, ei.attackCount[C])] / 100;

    return S(score, 0);
}


// Check if the board is (likely) drawn, logic from sjeng
//=================================================================
//! \brief  La position est-elle nulle ?
//!
//-----------------------------------------------------------------
bool Board::material_draw(void)
{
    //8/6R1/2k5/6P1/8/8/4nP2/6K1 w - - 1 41

    // Code de Vice (chap 82)
    //  >> vient de Sjeng 11.2 (draw.c) et neval.c , ligne 588
    //  >> qui vient de Faile

    // voir le code de Sjeng, qui comporte un test s'il reste des pions

    // No draw with pawns or queens
    if ( typePiecesBB[PieceType::Pawn] || typePiecesBB[PieceType::Queen])
        return false;

    // No rooks
    if (!typePiecesBB[PieceType::Rook])
    {
        // No bishops
        if (!typePiecesBB[PieceType::Bishop])
        {
            // Draw with 0-2 knights each (both 0 => KvK) (all nonpawns if any are knights)
            return(    non_pawn_count<WHITE>() <= 2
                   &&  non_pawn_count<BLACK>() <= 2);

        }

        // No knights
        else if (!typePiecesBB[PieceType::Knight])
        {
            // Draw unless one side has 2 extra bishops (all nonpawns are bishops)
            return( abs(  non_pawn_count<WHITE>()
                        - non_pawn_count<BLACK>()) < 2);

        }

        // Draw with 1-2 knights vs 1 bishop (there is at least 1 bishop, and at last 1 knight)
        else if (Bcount(typePiecesBB[PieceType::Bishop]) == 1)
        {
            if (Bcount(pieces_cp<WHITE, PieceType::Bishop>()) == 1)
                return non_pawn_count<WHITE>() == 1 && non_pawn_count<BLACK>() <= 2;
            else
                return non_pawn_count<BLACK>() == 1 && non_pawn_count<WHITE>() <= 2;
        }
    }

    // Draw with 1 rook + up to 1 minor each
    else if (   Bcount(pieces_cp<WHITE, PieceType::Rook>()) == 1
             && Bcount(pieces_cp<BLACK, PieceType::Rook>()) == 1)
    {
        return    non_pawn_count<WHITE>() <= 2
               && non_pawn_count<BLACK>() <= 2;
    }

    // Draw with 1 rook vs 1-2 minors
    else if ( Bcount(typePiecesBB[PieceType::Rook]) == 1)
    {
        if (Bcount(pieces_cp<WHITE, PieceType::Rook>()) == 1)
            return( non_pawn_count<WHITE>() == 1
                    && non_pawn_count<BLACK>() >= 1
                    && non_pawn_count<BLACK>() <= 2);
        else
            return( non_pawn_count<BLACK>() == 1
                && non_pawn_count<WHITE>() >= 1
                && non_pawn_count<WHITE>() <= 2);

    }

    return false;
}

template Score Board::evaluate_pawns<WHITE>(EvalInfo& ei);
template Score Board::evaluate_pawns<BLACK>(EvalInfo& ei);

template Score Board::evaluate_knights<WHITE>(EvalInfo& ei);
template Score Board::evaluate_knights<BLACK>(EvalInfo& ei);

template Score Board::evaluate_bishops<WHITE>(EvalInfo& ei);
template Score Board::evaluate_bishops<BLACK>(EvalInfo& ei);

template Score Board::evaluate_rooks<WHITE>(EvalInfo& ei);
template Score Board::evaluate_rooks<BLACK>(EvalInfo& ei);

template Score Board::evaluate_queens<WHITE>(EvalInfo& ei);
template Score Board::evaluate_queens<BLACK>(EvalInfo& ei);

template Score Board::evaluate_king<WHITE>(EvalInfo& ei);
template Score Board::evaluate_king<BLACK>(EvalInfo& ei);

template Score Board::evaluate_safety<WHITE>(const EvalInfo& ei);
template Score Board::evaluate_safety<BLACK>(const EvalInfo& ei);

