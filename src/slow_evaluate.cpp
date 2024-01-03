#include "defines.h"
#include "Board.h"
#include "evaluate.h"
#include "Attacks.h"
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
    Score eval = 0;
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
        Attacks::king_moves(x_king[BLACK]),
        Attacks::king_moves(x_king[WHITE])
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

#if defined USE_PAWN_CACHE

    Score pawn_eval = 0;
    // Recherche de la position des pions dans le cache
    if (transpositionTable.probe_pawn_table(pawn_hash, pawn_eval) == true)
    {
        // La table de pions contient la position,
        // On récupère le score de la table
        eval += pawn_eval;
    }
    else
    {
        // La table de pions ne contient pas la position,
        // on calcule le score, et on le stocke
        pawn_eval += evaluate_pawns<WHITE>(ei);
        pawn_eval -= evaluate_pawns<BLACK>(ei);
        transpositionTable.store_pawn_table(pawn_hash, pawn_eval);
        eval += pawn_eval;
    }
#else
    score += evaluate_pawns<WHITE>(ei);
    score -= evaluate_pawns<BLACK>(ei);
#endif

    //--------------------------------
    // Evaluation des pieces
    //--------------------------------

    eval += evaluate_knights<WHITE>(ei);
    eval -= evaluate_knights<BLACK>(ei);

    eval += evaluate_bishops<WHITE>(ei);
    eval -= evaluate_bishops<BLACK>(ei);

    eval += evaluate_rooks<WHITE>(ei);
    eval -= evaluate_rooks<BLACK>(ei);

    eval += evaluate_queens<WHITE>(ei);
    eval -= evaluate_queens<BLACK>(ei);

    eval += evaluate_king<WHITE>(ei);
    eval -= evaluate_king<BLACK>(ei);

    //--------------------------------
    // Evaluation de la sécurité du roi
    //--------------------------------

    eval += evaluate_safety<WHITE>(ei);
    eval -= evaluate_safety<BLACK>(ei);

    phase = ei.phase;
    return eval;
}

//=================================================================
//! \brief  Evaluation des pions d'une couleur
//-----------------------------------------------------------------
template<Color C>
Score Board::evaluate_pawns(EvalInfo& ei)
{
    int sq, sqpos;
    Score eval = 0;

    Bitboard pawns = ei.pawns[C];

    // Pions doublés
    // des pions doublés désignent deux pions de la même couleur sur une même colonne.
#if defined DEBUG_EVAL
            printf("Le camp %s possède %d pions doublés \n", side_name[Us].c_str(), Bcount(UsPawnsBB & north(UsPawnsBB)));
#endif
    eval += PawnDoubled * Bcount(pawns & north(pawns));

    // Pions supportés par un autre pion
#if defined DEBUG_EVAL
        printf("Le camp %s possède %d pions supportés par un autre pion \n", side_name[Us].c_str(), Bcount(UsPawnsBB & all_pawn_attacks<Us>(UsPawnsBB)));
#endif
    eval += PawnSupport * Bcount(pawns & all_pawn_attacks<C>(pawns));

    Bitboard bb = ei.pawns[C];
    while (bb)
    {
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(C, sq);              // case inversée pour les tables
        eval += meg_value[PieceType::Pawn];        // score matériel
        eval += meg_pawn_table[sqpos];             // score positionnel

#if defined DEBUG_EVAL
            //            printf("le pion %s (%d) sur la case %s a une valeur MG de %d \n", side_name[Us].c_str(), Us, square_name[sq].c_str(), mg_pawn_table[sqpos]);
#endif
        // Pion isolé
        //  Un pion isolé est un pion qui n'a plus de pion de son camp sur les colonnes adjacentes.
        //  Un pion isolé peut être redoutable en milieu de partie1.
        //  C'est souvent une faiblesse en finale, car il est difficile à défendre.
        if ((pawns & AdjacentFilesMask64[sq]) == 0)
        {
#if defined DEBUG_EVAL
            printf("le pion %s sur la case %s est isolé \n", side_name[Us].c_str(), square_name[sq].c_str());
#endif
            eval += PawnIsolated;
        }

        // Pion passé
        //  un pion passé est un pion qui n'est pas gêné dans son avance vers la 8e rangée par un pion adverse,
        //  c'est-à-dire qu'il n'y a pas de pion adverse devant lui, ni sur la même colonne, ni sur une colonne adjacente
        if ((passed_pawn_mask[C][sq] & ei.pawns[~C]) == 0)
        {
#if defined DEBUG_EVAL
            printf("le pion %s sur la case %s est passé \n", side_name[Us].c_str(), square_name[sq].c_str());
#endif
            eval += PawnPassed[Square::rank(sqpos)];
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

    return eval;
}

//=================================================================
//! \brief  Evaluation des cavaliers d'une couleur
//-----------------------------------------------------------------
template<Color C>
Score Board::evaluate_knights(EvalInfo& ei)
{
    int sq, sqpos;
    int count;
    Score eval = 0;

    Bitboard bb = ei.knights[C];
    ei.phase += Bcount(bb);

    while (bb)
    {
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(C, sq);              // case inversée pour les tables
        eval += meg_value[PieceType::Knight];      // score matériel
        eval += meg_knight_table[sqpos];           // score positionnel

        // mobilité
        Bitboard mobilityBB = Attacks::knight_moves(sq) & ei.mobilityArea[C];
        count = Bcount(mobilityBB);
        eval += KnightMobility[count];

        // Un avant-poste est une case sur laquelle votre pièce ne peut pas
        // être attaquée par un pion adverse.
        // L’avant-poste est particulièrement efficace quand il est défendu
        // par un de vos propres pions et qu’il permet de contrôler des cases clefs
        // de la position adverse.
        // Les cavaliers sont les pièces qui profitent le mieux des avant-postes.

        // Apply a bonus if the knight is on an outpost square, and cannot be attacked
        // by an enemy pawn. Increase the bonus if one of our pawns supports the knight.

        // définition sujette à discussion ....
#if 0
        if ( (OutpostRanks[C] & (square_to_bit(sq))
             && !(outpost_mask[C][sq] & ei.pawns[~C]) ) )
        {
            int defended = !!(ei.pawnAttacks[C] & square_to_bit(sq));


            score += Score(KnightOutpost[defended]);
#if defined DEBUG_EVAL
            printf("le cavalier %s sur la case %s est sur un avant-poste défendu %d fois", side_name[Us].c_str(), square_name[sq].c_str(), defended);
#endif
            if (defended)
                printf("le cavalier %s sur la case %s est sur un avant-poste défendu \n", side_name[C].c_str(), square_name[sq].c_str());
            else
                printf("le cavalier %s sur la case %s est sur un avant-poste non défendu \n", side_name[C].c_str(), square_name[sq].c_str());
        }
#endif
        //  attaques et échecs pour calculer la sécurité du roi
        int nbr_attacks = Bcount(mobilityBB & ei.enemyKingZone[C]);
        int nbr_checks  = Bcount(mobilityBB & Attacks::knight_moves(x_king[~C]));

        if (nbr_attacks > 0 || nbr_checks > 0)
        {
            ei.attackCount[C]++;
            ei.attackPower[C] += nbr_attacks * AttackPower[PieceType::Knight] + nbr_checks * CheckPower[PieceType::Knight];
        }
    }
    return eval;
}

//=================================================================
//! \brief  Evaluation des fous d'une couleur
//-----------------------------------------------------------------
template<Color C>
Score Board::evaluate_bishops(EvalInfo& ei)
{
    int sq, sqpos;
    int count;
    Score eval = 0;
    Bitboard bb = ei.bishops[C];
    ei.phase += Bcount(bb);

    if (Bcount(bb) >= 2)
    {
        // Paire de fous
        // On ne teste pas si les fous sont de couleur différente !
#if defined DEBUG_EVAL
            printf("Le camp %s possède la paire de fous \n", side_name[Us].c_str());
#endif
        eval += BishopPair;
    }

    while (bb)
    {
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(C, sq);              // case inversée pour les tables
        eval += meg_value[PieceType::Bishop];      // score matériel
        eval += meg_bishop_table[sqpos];           // score positionnel

        // mobilité
        Bitboard mobilityBB = XRayBishopAttack<C>(sq) & ei.mobilityArea[C];
        count = Bcount(mobilityBB);
        eval += BishopMobility[count];

        //  attaques et échecs pour calculer la sécurité du roi
        int nbr_attacks = Bcount(mobilityBB & ei.enemyKingZone[C]);
        int nbr_checks  = Bcount(mobilityBB & Attacks::bishop_moves(x_king[~C], ei.occupiedBB));

        if (nbr_attacks > 0 || nbr_checks > 0)
        {
            ei.attackCount[C] ++;
            ei.attackPower[C] += nbr_attacks * AttackPower[PieceType::Bishop] + nbr_checks * CheckPower[PieceType::Bishop];
        }
    }
    return eval;
}

//=================================================================
//! \brief  Evaluation des tours d'une couleur
//-----------------------------------------------------------------
template<Color C>
Score Board::evaluate_rooks(EvalInfo& ei)
{
    int sq, sqpos;
    int count;
    Score eval = 0;

    Bitboard bb = ei.rooks[C];
    ei.phase += 2*Bcount(bb);

    while (bb)
    {
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(C, sq);              // case inversée pour les tables
        eval += meg_value[PieceType::Rook];    // score matériel
        eval += meg_rook_table[sqpos];            // score positionnel

        // mobilité
        Bitboard mobilityBB = XRayRookAttack<C>(sq) & ei.mobilityArea[C];
        count = Bcount(mobilityBB);
        eval += RookMobility[count];

        //  attaques et échecs pour calculer la sécurité du roi
        int nbr_attacks = Bcount(mobilityBB & ei.enemyKingZone[C]);
        int nbr_checks  = Bcount(mobilityBB & Attacks::rook_moves(x_king[~C], ei.occupiedBB));

        if (nbr_attacks > 0 || nbr_checks > 0)
        {
            ei.attackCount[C]++;
            ei.attackPower[C] += nbr_attacks * AttackPower[PieceType::Rook] + nbr_checks * CheckPower[PieceType::Rook];
        }

        // Colonnes ouvertes
        if (((ei.pawns[WHITE] | ei.pawns[BLACK]) & FileMask64[sq]) == 0) // pas de pion ami ou ennemi
        {
#if defined DEBUG_EVAL
                printf("la tour %s sur la case %s est sur une colonne ouverte \n",
                       side_name[Us].c_str(), square_name[sq].c_str());
#endif
            eval += RookOpenFile;
        }

        // Colonnes semi-ouvertes
        else if ((ei.pawns[C] & FileMask64[sq]) == 0) // pas de pion ami
        {
#if defined DEBUG_EVAL
                printf("la tour %s sur la case %s est sur une colonne semi-ouverte \n",
                       side_name[Us].c_str(), square_name[sq].c_str());
#endif
            eval += RookSemiOpenFile;
        }

    }
    return eval;
}

//=================================================================
//! \brief  Evaluation des reines d'une couleur
//-----------------------------------------------------------------
template<Color C>
Score Board::evaluate_queens(EvalInfo& ei)
{
    int sq, sqpos;
    int count;
    Score eval = 0;
    Bitboard bb = ei.queens[C];
    ei.phase += 4*Bcount(bb);

    while (bb)
    {
        sq     = next_square(bb);                   // case où est la pièce
        sqpos  = Square::flip(C, sq);              // case inversée pour les tables
        eval += meg_value[PieceType::Queen];    // score matériel
        eval += meg_queen_table[sqpos];            // score positionnel

        // mobilité
        Bitboard mobilityBB = XRayQueenAttack<C>(sq) & ei.mobilityArea[C];
        count = Bcount(mobilityBB);
        eval += QueenMobility[count];

        //  attaques et échecs pour calculer la sécurité du roi

        // nbr_attacks : nombre d'attaques sur la zone du roi ennemi
        //          à noter que seules les cases de la mobilité comptent
        int nbr_attacks = Bcount(mobilityBB & ei.enemyKingZone[C]);

        // nbr_checks = nombre de cases d'où la dame peut faire échec
        //          à noter que seules les cases de la mobilité comptent
        int nbr_checks     = Bcount(mobilityBB & Attacks::queen_moves(x_king[~C], ei.occupiedBB));

        if (nbr_attacks > 0 || nbr_checks > 0)
        {
            ei.attackCount[C]++;
            ei.attackPower[C] += nbr_attacks * AttackPower[PieceType::Queen]
                               + nbr_checks  * CheckPower[PieceType::Queen];
        }

        // Colonnes ouvertes
        if (((ei.pawns[WHITE] | ei.pawns[BLACK]) & FileMask64[sq]) == 0) // pas de pion ami ou ennemi
        {
#if defined DEBUG_EVAL
            printf("la Dame %s sur la case %s est sur une colonne ouverte \n",
                   side_name[Us].c_str(), square_name[sq].c_str());
#endif
            eval += QueenOpenFile;
        }

        // Colonnes semi-ouvertes
        else if ((ei.pawns[C] & FileMask64[sq]) == 0) // pas de pion ami
        {
#if defined DEBUG_EVAL
            printf("la dame %s sur la case %s est sur une colonne semi-ouverte \n",
                   side_name[Us].c_str(), square_name[sq].c_str());
#endif
            eval += QueenSemiOpenFile;
        }
    }
    return eval;
}

//=================================================================
//! \brief  Evaluation du roi d'une couleur
//-----------------------------------------------------------------
template<Color C>
Score Board::evaluate_king(EvalInfo& ei)
{
    int sq, sqpos;
    Score eval = 0;
    sq     = x_king[C];
    sqpos  = Square::flip(C, sq);              // case inversée pour les tables
    eval += meg_value[PieceType::King];       // score matériel
    eval += meg_king_table[sqpos];            // score positionnel

    // sécurité du roi
    // On compte le nombre de case d'où on peut attaquer le roi (sauf cavalier)
    Bitboard SafeLine = RankMask8[Square::relative_rank<C>(RANK_1)];
    int count = Bcount(~SafeLine & Attacks::queen_moves(sq, colorPiecesBB[C] | typePiecesBB[PieceType::Pawn]));
    eval += KingLineDanger[count];

    // Add to enemy's attack power based on open lines
    ei.attackPower[~C] += (count - 3) * 8;

    return eval;
}

//=================================================================
//! \brief  Evaluation de la sécurité du roi
//-----------------------------------------------------------------
template<Color C>
Score Board::evaluate_safety(const EvalInfo& ei)
{
    int eval = ei.attackPower[C] * CountModifier[std::min(7, ei.attackCount[C])] / 100;

    return S(eval, 0);
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

