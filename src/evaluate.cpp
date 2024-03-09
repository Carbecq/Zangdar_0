#include "Board.h"
#include "defines.h"
#include "evaluate.h"
#include "TranspositionTable.h"

#if defined USE_TUNER
#include "Tuner.h"
#endif

/*
https://github.com/nmrugg/stockfish.js/blob/master/src/evaluate.cpp

https://hxim.github.io/Stockfish-Evaluation-Guide/
*/


//==========================================
//! \brief  Evaluation de la position
//! \return Evaluation statique de la position
//! du point de vue du camp au trait.
//------------------------------------------
[[nodiscard]] Score Board::evaluate()
{
    Score eval = 0;
    EvalInfo ei;

    // nullité
    // On utilise les tables Syzygy, ce test de nullité est inutile

    // voir le code de Sjeng (aussi Fruit-Mora), qui comporte un test s'il reste des pions
//    if (material_draw() == true)
//        return 0;

    // Initialisations
    init_eval_info(ei);

    //--------------------------------
    //  Evaluation des pions
    //--------------------------------
    eval += probe_pawn_cache(ei);

    //--------------------------------
    // Evaluation des pieces
    //--------------------------------
    eval += evaluate_pieces(ei);

    //--------------------------------
    // Evaluation des pions passés
    //--------------------------------
    eval += evaluate_passed<WHITE>(ei)  - evaluate_passed<BLACK>(ei);

    //--------------------------------
    // Evaluation des menaces
    //--------------------------------
    eval += evaluate_threats<WHITE>(ei) - evaluate_threats<BLACK>(ei);

#if defined USE_TUNER
    ownTuner.Trace.eval = eval;
#endif

    // Adjust eval by scale factor
    int scale = scale_factor(eval);
#if defined USE_TUNER
    ownTuner.Trace.scale = scale;
#endif

    // Adjust score by phase
    int phase24  = ei.phase24;
    int phase256 = get_phase256(phase24);

    // Compute and store an interpolated evaluation from white's POV
    int intereval = ( MgScore(eval) * phase256
         +   EgScore(eval) * (256 - phase256) * scale / 128 ) / 256;

    // Return the evaluation, negated if we are black
    return (side_to_move == WHITE ? intereval : -intereval) + Tempo;
}

//=======================================================
//! \brief  Calcul ou récupération de l'évaluation des pions
//-------------------------------------------------------
Score Board::probe_pawn_cache(EvalInfo& ei)
{
    // On ne peut pas utiliser la table lors d'un tuning
#if defined USE_TUNER
    return (evaluate_pawns<WHITE>(ei) - evaluate_pawns<BLACK>(ei));
#else

    Score eval = 0;
    Bitboard passed_pawns;

    // Recherche de la position des pions dans le cache
    if (transpositionTable.probe_pawn_table(pawn_hash, eval, passed_pawns) == true)
    {
        // La table de pions contient la position,
        // On récupère l'évaluation de la table
        ei.passedPawns = passed_pawns;
    }
    else
    {
        // La table de pions ne contient pas la position,
        // on calcule l'évaluation, et on la stocke
        eval = evaluate_pawns<WHITE>(ei) - evaluate_pawns<BLACK>(ei);
        transpositionTable.store_pawn_table(pawn_hash, eval, ei.passedPawns);   //TODO passedPawns = ???
    }

    return eval;

#endif

}

//===================================================================
//! \brief  Evaluation des pièces
//-------------------------------------------------------------------
Score Board::evaluate_pieces(EvalInfo& ei)
{
    Score eval = 0;

    eval += evaluate_knights<WHITE>(ei) - evaluate_knights<BLACK>(ei);
    eval += evaluate_bishops<WHITE>(ei) - evaluate_bishops<BLACK>(ei);
    eval += evaluate_rooks<WHITE>(ei)   - evaluate_rooks<BLACK>(ei);
    eval += evaluate_queens<WHITE>(ei)  - evaluate_queens<BLACK>(ei);
    eval += evaluate_king<WHITE>(ei)    - evaluate_king<BLACK>(ei);

    return eval;
}

//=================================================================
//! \brief  Evaluation des pions d'une couleur
//!
//-----------------------------------------------------------------
template<Color US>
Score Board::evaluate_pawns(EvalInfo& ei)
{
    constexpr Color THEM = ~US;
    constexpr Direction DOWN = (US == WHITE) ? SOUTH : NORTH;

    int sq, sqpos;
    int count;
    int rank;
    Score eval = 0;

    Bitboard pawns = ei.pawns[US];

    // https://chessfox.com/pawn-structures-why-pawns-are-the-soul-of-chess/

    // isolés (voir ethereal)
    // double isolated  : 2 pions doublés bloqués par un pion ennemi
    // backward
    // doubled
    // connected
    // Weak unopposed pawn.
    // blocked

    // Pions doublés
    // 1- des pions doublés désignent deux pions de la même couleur sur une même colonne.
    // 2- n'est pris en compte que si l'un empèche l'autre de bouger
    count = BB::count_bit(pawns & BB::north(pawns));
    eval += PawnDoubled * count;

#if defined DEBUG_EVAL
    if (count)
        printf("Le camp %s possède %d pions doublés \n", camp[1][US].c_str(), count);
#endif
#if defined USE_TUNER
    ownTuner.Trace.PawnDoubled[US] = count;
#endif

    // Pions liés
    // Des pions sont liés lorsqu'ils sont placés sur une diagonale et que l'un défend l'autre.
    count = BB::count_bit(pawns & all_pawn_attacks<THEM>(pawns));
    eval += PawnSupport * count;

#if defined DEBUG_EVAL
    if (count)
        printf("Le camp %s possède %d pions liés \n", camp[1][US].c_str(), count);
#endif
#if defined USE_TUNER
    ownTuner.Trace.PawnSupport[US] = count;
#endif

    // Open pawns
    // undefended, unopposed pawns (#436)
    // Undefended, unopposed pawns are targets for constant pressure by rooks and queens.
    Bitboard open = ~BB::fill<DOWN>(ei.pawns[THEM]);
    count = BB::count_bit(pawns & open & ~all_pawn_attacks<US>(pawns));
    eval += PawnOpen * count;

#if defined DEBUG_EVAL
    if (count)
        printf("Le camp %s possède %d pions non défendus sans opposition \n", camp[1][US].c_str(), count);
#endif
#if defined USE_TUNER
    ownTuner.Trace.PawnOpen[US] = count;
#endif

    // Pions en phalange (Pawn Phalanx)
    Bitboard phalanx = pawns & BB::west(pawns);
    while (phalanx)
    {
        rank  = SQ::relative_rank8<US>(SQ::rank(BB::pop_lsb(phalanx)));
        eval += PawnPhalanx[rank];

#if defined DEBUG_EVAL
        printf("Le camp %s possède des pions en phalange sur la rangée %d \n", camp[1][US].c_str(), rank);
#endif
#if defined USE_TUNER
        ownTuner.Trace.PawnPhalanx[rank][US]++;
#endif
    }

    // Evaluation des pions un par un
    Bitboard bb = ei.pawns[US];
    while (bb)
    {
        sq    = BB::pop_lsb(bb);                   // case où est la pièce
        sqpos = SQ::relative_square<US>(sq);               // case inversée pour les tables
        eval += PawnValue;                         // score matériel
        eval += PawnPSQT[sqpos];                   // score positionnel

#if defined DEBUG_EVAL
            //            printf("le pion %s (%d) sur la case %s a une valeur MG de %d \n", side_name[US].c_str(), Us, square_name[sq].c_str(), mg_pawn_table[sqpos]);
#endif
#if defined USE_TUNER
        ownTuner.Trace.PawnValue[US]++;
        ownTuner.Trace.PawnPSQT[sqpos][US]++;
#endif

        // Pion isolé
        //  Un pion isolé est un pion qui n'a plus de pion de son camp sur les colonnes adjacentes.
        //  Un pion isolé peut être redoutable en milieu de partie1.
        //  C'est souvent une faiblesse en finale, car il est difficile à défendre.
        if ((pawns & AdjacentFilesMask64[sq]) == 0)
        {
            eval += PawnIsolated;
#if defined DEBUG_EVAL
            printf("le pion %s en %s est isolé \n", camp[1][US].c_str(), square_name[sq].c_str());
#endif
#if defined USE_TUNER
            ownTuner.Trace.PawnIsolated[US]++;
#endif
        }

        // Pion passé
        //  un pion passé est un pion qui n'est pas gêné dans son avance vers la 8e rangée par un pion adverse,
        //  c'est-à-dire qu'il n'y a pas de pion adverse devant lui, ni sur la même colonne, ni sur une colonne adjacente
        if ((PassedPawnMask[US][sq] & ei.pawns[THEM]) == 0)
        {
            rank  = SQ::rank(sqpos);
            eval += PawnPassed[rank];

#if defined DEBUG_EVAL
            printf("le pion %s en %s est passé \n", camp[1][US].c_str(), square_name[sq].c_str());
#endif
#if defined USE_TUNER
            ownTuner.Trace.PawnPassed[rank][US]++;
#endif

            // Pion passé protégé
            if (BB::sq2BB(sq) & all_pawn_attacks<US>(pawns))
            {
                eval += PassedDefended[rank];

#if defined DEBUG_EVAL
                printf("le pion %s en %s est passé et protégé \n", camp[1][US].c_str(), square_name[sq].c_str());
#endif
#if defined USE_TUNER
                ownTuner.Trace.PassedDefended[rank][US]++;
#endif
            }

            ei.passedPawns |= BB::sq2BB(sq);
        }

    } // pions

    return eval;
}

//=================================================================
//! \brief  Evaluation des pions passés d'une couleur
//-----------------------------------------------------------------
template<Color US>
Score Board::evaluate_passed(EvalInfo& ei)
{
    constexpr Color THEM     = ~US;
    constexpr Direction UP   = (US == WHITE) ? NORTH : SOUTH;

    Score eval = 0;
    int count;
    int sq;
    int forward;

    Bitboard passers = colorPiecesBB[US] & ei.passedPawns;

    while (passers)
    {
        sq     = BB::pop_lsb(passers);                   // case où est la pièce

        forward = sq + UP;
        int rank = SQ::relative_rank8<US>(SQ::rank(sq));
        int r = rank - RANK_4;

        if (rank < RANK_4)
            continue;

        // Distance au roi ami
        count = distance_between(forward, king_square<US>());
        eval += count * PassedDistUs[r];

#if defined DEBUG_EVAL
        if (count)
            printf("le pion passé %s en %s éloigné du roi ami de %d \n", camp[1][US].c_str(), square_name[sq].c_str(), count);
#endif
#if defined USE_TUNER
        ownTuner.Trace.PassedDistUs[r][US] += count;
#endif

        // Distance au roi ennemi
        count = (rank - RANK_3) * distance_between(forward, king_square<THEM>());
        eval += count * PassedDistThem;

#if defined DEBUG_EVAL
        if (count)
            printf("le pion passé %s en %s éloigné du roi ennemi de %d \n", camp[1][US].c_str(), square_name[sq].c_str(), count);
#endif
#if defined USE_TUNER
        ownTuner.Trace.PassedDistThem[US] += count;
#endif

        // Pion passé bloqué
        if (pieceOn[forward])
        {
            eval += PassedBlocked[r];

#if defined DEBUG_EVAL
            printf("le pion passé %s en %s est bloqué \n", camp[1][US].c_str(), square_name[sq].c_str());
#endif
#if defined USE_TUNER
            ownTuner.Trace.PassedBlocked[r][US]++;
#endif
        }
    }

    return eval;
}

//=================================================================
//! \brief  Evaluation des cavaliers d'une couleur
//-----------------------------------------------------------------
template<Color US>
Score Board::evaluate_knights(EvalInfo& ei)
{
    constexpr Color THEM = ~US;
    constexpr Direction DOWN = (US == WHITE) ? SOUTH : NORTH;

    int sq, sqpos;
    int count;
    Score eval = 0;
    int defended;

    Bitboard bb = ei.knights[US];
    ei.phase24 += BB::count_bit(bb);
    Bitboard     enemyPawns = ei.pawns[THEM];

    // Le cavalier est protégé par un pion
    // Il ne peut pas être attaqué par une tour
    count = BB::count_bit(bb & BB::shift<DOWN>(typePiecesBB[PAWN]));
    eval += count * MinorBehindPawn;

#if defined DEBUG_EVAL
    if (count)
        printf("%d cavaliers %s sont situés derrière un pion \n", count, camp[1][US].c_str());
#endif
#if defined USE_TUNER
    ownTuner.Trace.MinorBehindPawn[US] = count;
#endif

    while (bb)
    {
        sq    = BB::pop_lsb(bb);                   // case où est la pièce
        sqpos = SQ::relative_square<US>(sq);           // case inversée pour les tables
        eval += KnightValue;                    // score matériel
        eval += KnightPSQT[sqpos];              // score positionnel

#if defined USE_TUNER
        ownTuner.Trace.KnightValue[US]++;
        ownTuner.Trace.KnightPSQT[sqpos][US]++;
#endif


        // Un avant-poste est une case sur laquelle votre pièce ne peut pas
        // être attaquée par un pion adverse.
        // L’avant-poste est particulièrement efficace quand il est défendu
        // par un de vos propres pions et qu’il permet de contrôler des cases clefs
        // de la position adverse.
        // Les cavaliers sont les pièces qui profitent le mieux des avant-postes.

        // Ajoute un bonus si le cavalier est sur un avant-poste,
        // et ne peut pas être attaqué par un pion ennemi.
        // Augmente ce bonus si le cavalier est protégé par un de nos pions.

        // TODO : a) être sur une colonne semi-ouverte
        //        b) être sur une des colonnes C,D,E,F

        if (    BB::test_bit(OutpostRanksMasks[US], sq)
            && !(OutpostSquareMasks[US][sq] & enemyPawns))
        {
            defended = BB::test_bit(ei.pawnAttacks[US], sq);
            eval += KnightOutpost[defended];

#if defined DEBUG_EVAL
            if (defended)
                printf("le cavalier %s en %s est sur un avant-poste défendu \n", camp[1][US].c_str(), square_name[sq].c_str());
            else
                printf("le cavalier %s en %s est sur un avant-poste non défendu \n", camp[1][US].c_str(), square_name[sq].c_str());
#endif
#if defined USE_TUNER
            ownTuner.Trace.KnightOutpost[defended][US]++;
#endif
        }


        // mobilité
        Bitboard mobilityBB = Attacks::knight_moves(sq) & ei.mobilityArea[US];
        count = BB::count_bit(mobilityBB);
        eval += KnightMobility[count];
#if defined USE_TUNER
        ownTuner.Trace.KnightMobility[count][US]++;
#endif

        //  attaques et échecs pour calculer la sécurité du roi
        int nbr_attacks = BB::count_bit(mobilityBB & ei.enemyKingZone[US]);
        int nbr_checks  = BB::count_bit(mobilityBB & Attacks::knight_moves(x_king[THEM]));

        if (nbr_attacks > 0 || nbr_checks > 0)
        {
            ei.attackCount[US]++;
            ei.attackPower[US] += nbr_attacks * AttackPower[KNIGHT] + nbr_checks * CheckPower[KNIGHT];
        }
    }

    return eval;
}

//=================================================================
//! \brief  Evaluation des fous d'une couleur
//-----------------------------------------------------------------
template<Color US>
Score Board::evaluate_bishops(EvalInfo& ei)
{
    constexpr Color THEM = ~US;
    constexpr Direction DOWN = (US == WHITE) ? SOUTH : NORTH;

    int sq, sqpos;
    int count;

    Bitboard bishopSquares;
    Bitboard badPawns;          // pions de la couleur du fou
    Bitboard blockedPawns;

    Score    eval = 0;
    Bitboard bb   = ei.bishops[US];
    ei.phase24 += BB::count_bit(bb);

    // Paire de fous
    // On ne teste pas si les fous sont de couleur différente !
    if (BB::count_bit(bb) >= 2)
    {
        eval += BishopPair;
#if defined DEBUG_EVAL
        printf("Le camp %s possède la paire de fous \n", side_name[US].c_str());
#endif
#if defined USE_TUNER
        ownTuner.Trace.BishopPair[US]++;
#endif
    }

    // Le fou est protégé par un pion
    // Il ne peut pas être attaqué par une tour
    count = BB::count_bit(bb & BB::shift<DOWN>(typePiecesBB[PAWN]));
    eval += count * MinorBehindPawn;

#if defined DEBUG_EVAL
    if (count)
        printf("%d fous %s sont situés derrière un pion \n", count, camp[1][US].c_str());
#endif
#if defined USE_TUNER
    ownTuner.Trace.MinorBehindPawn[US] = count;
#endif

    while (bb)
    {
        sq    = BB::pop_lsb(bb);                   // case où est la pièce
        sqpos = SQ::relative_square<US>(sq);               // case inversée pour les tables
        eval += BishopValue;                       // score matériel
        eval += BishopPSQT[sqpos];                 // score positionnel

#if defined USE_TUNER
        ownTuner.Trace.BishopValue[US]++;
        ownTuner.Trace.BishopPSQT[sqpos][US]++;
#endif

        // Mauvais fou
        // https://hxim.github.io/Stockfish-Evaluation-Guide/
        // Bishop pawns. Number of pawns on the same color square as the bishop multiplied by one plus the number of our blocked pawns in the center files C, D, E or F.
        bishopSquares = (BB::sq2BB(sq) & DarkSquares) ? DarkSquares : LightSquares;       // Bitboard de la couleur du fou
        badPawns      = ei.pawns[US] & bishopSquares;                                     // Bitboard de nos pions de la couleur du fou
        blockedPawns  = BB::shift<DOWN>(occupancy_all()) & ei.pawns[US] & CenterFiles;    // Bitboard de nos pions centraux bloqués

        count = BB::count_bit(badPawns) * (1 + BB::count_bit(blockedPawns));
        eval += count * BishopBadPawn;

#if defined DEBUG_EVAL
    if (count)
        printf("le fou %s en %s a %d mauvais pions et %d pions bloquants \n", camp[1][US].c_str(), square_name[sq].c_str(),
                BB::count_bit(badPawns), BB::count_bit(blockedPawns));
#endif
#if defined USE_TUNER
    ownTuner.Trace.BishopBadPawn[US] += count;
#endif

        // mobilité
        Bitboard mobilityBB = XRayBishopAttack<US>(sq) & ei.mobilityArea[US];
        count = BB::count_bit(mobilityBB);
        eval += BishopMobility[count];
#if defined USE_TUNER
        ownTuner.Trace.BishopMobility[count][US]++;
#endif

        //  attaques et échecs pour calculer la sécurité du roi
        int nbr_attacks = BB::count_bit(mobilityBB & ei.enemyKingZone[US]);
        int nbr_checks  = BB::count_bit(mobilityBB & Attacks::bishop_moves(x_king[THEM], ei.occupied));

        if (nbr_attacks > 0 || nbr_checks > 0)
        {
            ei.attackCount[US] ++;
            ei.attackPower[US] += nbr_attacks * AttackPower[BISHOP] + nbr_checks * CheckPower[BISHOP];
        }
    }

    return eval;
}

//=================================================================
//! \brief  Evaluation des tours d'une couleur
//-----------------------------------------------------------------
template<Color US>
Score Board::evaluate_rooks(EvalInfo& ei)
{
    constexpr Color THEM = ~US;

    int sq, sqpos;
    int count;
    Score eval = 0;

    Bitboard bb = ei.rooks[US];
    ei.phase24 += 2*BB::count_bit(bb);

    while (bb)
    {
        sq     = BB::pop_lsb(bb);                   // case où est la pièce
        sqpos  = SQ::relative_square<US>(sq);               // case inversée pour les tables
        eval += RookValue;                         // score matériel
        eval += RookPSQT[sqpos];                   // score positionnel

#if defined USE_TUNER
        ownTuner.Trace.RookValue[US]++;
        ownTuner.Trace.RookPSQT[sqpos][US]++;
#endif

        // Colonnes ouvertes
        if (((ei.pawns[WHITE] | ei.pawns[BLACK]) & FileMask64[sq]) == 0) // pas de pion ami ou ennemi
        {
            eval += OpenForward;

#if defined DEBUG_EVAL
            printf("la tour %s en %s est sur une colonne ouverte \n",
                   camp[0][US].c_str(), square_name[sq].c_str());
#endif
#if defined USE_TUNER
            ownTuner.Trace.OpenForward[US]++;
#endif
        }
        // Colonnes semi-ouvertes
        // ne pas ajouter les 2 bonus
        else if ((ei.pawns[US] & FileMask64[sq]) == 0) // pas de pion ami
        {
            eval += SemiForward;

#if defined DEBUG_EVAL
            printf("la tour %s en %s est sur une colonne semi-ouverte \n",
                       camp[0][US].c_str(), square_name[sq].c_str());
#endif
#if defined USE_TUNER
            ownTuner.Trace.SemiForward[US]++;
#endif
        }

        // Ethereal 14
        // Rook gains a bonus for being located on seventh rank relative to its
        // colour so long as the enemy king is on the last two ranks of the board
        // if (   relativeRankOf(US, sq) == 6
        //     && relativeRankOf(US, ei->kingSquare[THEM]) >= 6) {
        //     eval += RookOnSeventh;
        //     if (TRACE) T.RookOnSeventh[US]++;
        // }

        // mobilité
        Bitboard mobilityBB = XRayRookAttack<US>(sq) & ei.mobilityArea[US];
        count = BB::count_bit(mobilityBB);
        eval += RookMobility[count];

#if defined USE_TUNER
        ownTuner.Trace.RookMobility[count][US]++;
#endif

        //  attaques et échecs pour calculer la sécurité du roi
        int nbr_attacks = BB::count_bit(mobilityBB & ei.enemyKingZone[US]);
        int nbr_checks  = BB::count_bit(mobilityBB & Attacks::rook_moves(x_king[THEM], ei.occupied));

        if (nbr_attacks > 0 || nbr_checks > 0)
        {
            ei.attackCount[US]++;
            ei.attackPower[US] += nbr_attacks * AttackPower[ROOK] + nbr_checks * CheckPower[ROOK];
        }
    }

    return eval;
}

//=================================================================
//! \brief  Evaluation des reines d'une couleur
//-----------------------------------------------------------------
template<Color US>
Score Board::evaluate_queens(EvalInfo& ei)
{
    constexpr Color THEM = ~US;

    int sq, sqpos;
    int count;
    Score eval = 0;
    Bitboard bb = ei.queens[US];
    ei.phase24 += 4*BB::count_bit(bb);

    while (bb)
    {
        sq     = BB::pop_lsb(bb);                   // case où est la pièce
        sqpos  = SQ::relative_square<US>(sq);               // case inversée pour les tables
        eval += QueenValue;                        // score matériel
        eval += QueenPSQT[sqpos];                  // score positionnel

#if defined USE_TUNER
        ownTuner.Trace.QueenValue[US]++;
        ownTuner.Trace.QueenPSQT[sqpos][US]++;
#endif

        // mobilité
        Bitboard mobilityBB = XRayQueenAttack<US>(sq) & ei.mobilityArea[US];
        count = BB::count_bit(mobilityBB);
        eval += QueenMobility[count];

#if defined USE_TUNER
        ownTuner.Trace.QueenMobility[count][US]++;
#endif

        //  attaques et échecs pour calculer la sécurité du roi

        // nbr_attacks : nombre d'attaques sur la zone du roi ennemi
        //          à noter que seules les cases de la mobilité comptent
        int nbr_attacks = BB::count_bit(mobilityBB & ei.enemyKingZone[US]);

        // nbr_checks = nombre de cases d'où la dame peut faire échec
        //          à noter que seules les cases de la mobilité comptent
        int nbr_checks     = BB::count_bit(mobilityBB & Attacks::queen_moves(x_king[THEM], ei.occupied));

        if (nbr_attacks > 0 || nbr_checks > 0)
        {
            ei.attackCount[US]++;
            ei.attackPower[US] += nbr_attacks * AttackPower[QUEEN]
                                 + nbr_checks  * CheckPower[QUEEN];
        }
    }

    return eval;
}

//=================================================================
//! \brief  Evaluation du roi d'une couleur
//-----------------------------------------------------------------
template<Color US>
Score Board::evaluate_king(EvalInfo& ei)
{
    constexpr Color THEM = ~US;

    int sq, sqpos;
    Score eval = 0;
    sq     = x_king[US];
    sqpos  = SQ::relative_square<US>(sq);               // case inversée pour les tables
    eval += KingValue;                         // score matériel
    eval += KingPSQT[sqpos];                   // score positionnel

#if defined USE_TUNER
    ownTuner.Trace.KingPSQT[sqpos][US]++;
#endif

    // sécurité du roi
    // On compte le nombre de case d'où on peut attaquer le roi (sauf cavalier)
    Bitboard SafeLine = RankMask8[SQ::relative_rank8<US>(RANK_1)];
    
    int count = BB::count_bit(~SafeLine & Attacks::queen_moves(sq, colorPiecesBB[US] | typePiecesBB[PAWN]));

    // if (min_count == -1)
    //     min_count = count;
    // else
    //     min_count = std::min(min_count, count);

    // if (max_count == -1)
    //     max_count = count;
    // else
    //     max_count = std::max(max_count, count);

    eval += KingLineDanger[count];
#if defined USE_TUNER
    ownTuner.Trace.KingLineDanger[count][US]++;
#endif

    // King threatening a pawn
    if (Attacks::king_moves(sq) & occupancy_cp<THEM, PAWN>())
    {
        eval += KingAtkPawn;

#if defined USE_TUNER
        ownTuner.Trace.KingAtkPawn[US]++;
#endif
    }

    // Add to enemy's attack power based on open lines
    ei.attackPower[THEM] += (count - 3) * 8;

    // Evaluation de la sécurité du roi
    int danger =  ei.attackPower[THEM]
                 * CountModifier[std::min(7, ei.attackCount[THEM])];

    eval -= S(danger / 128, 0);

    return eval;
}

//=================================================================
//! \brief  Evaluation des menaces de pions
//-----------------------------------------------------------------
template<Color US>
Score Board::evaluate_threats(const EvalInfo& ei)
{
    constexpr Color THEM = ~US;
    constexpr Direction UP = (US == WHITE) ? NORTH : SOUTH;

    int count;
    Score eval = 0;

    Bitboard ourPawns      = ei.pawns[US];
    Bitboard theirNonPawns = colorPiecesBB[THEM] ^ occupancy_cp<THEM, PAWN>();
    
    count = BB::count_bit(all_pawn_attacks<US>(ourPawns) & theirNonPawns);
    eval += PawnThreat * count;
#if defined USE_TUNER
    ownTuner.Trace.PawnThreat[US] = count;
#endif

    Bitboard pawnPushes = BB::shift<UP>(ourPawns) & ~ei.occupied;
        //   ShiftBB(ourPawns, up) & ~pieceBB(ALL);
    
    count = BB::count_bit(all_pawn_attacks<US>(pawnPushes) & theirNonPawns);
    eval += PushThreat * count;
#if defined USE_TUNER
    ownTuner.Trace.PushThreat[US] = count;
#endif

    return eval;
}


//=================================================================
//! \brief  La position est-elle nulle ?
//!  Check if the board is (likely) drawn, logic from sjeng
//-----------------------------------------------------------------
bool Board::material_draw(void)
{
    //8/6R1/2k5/6P1/8/8/4nP2/6K1 w - - 1 41

    // Code de Vice (chap 82)
    //  >> vient de Sjeng 11.2 (draw.c) et neval.c , ligne 588
    //  >> qui vient de Faile

    // voir le code de Sjeng, qui comporte un test s'il reste des pions

    // No draw with pawns or queens
    if ( typePiecesBB[PAWN] || typePiecesBB[QUEEN])
        return false;

    // No rooks
    if (!typePiecesBB[ROOK])
    {
        // No bishops
        if (!typePiecesBB[BISHOP])
        {
            // Draw with 0-2 knights each (both 0 => KvK) (all nonpawns if any are knights)
            return(    non_pawn_count<WHITE>() <= 2
                   &&  non_pawn_count<BLACK>() <= 2);

        }

        // No knights
        else if (!typePiecesBB[KNIGHT])
        {
            // Draw unless one side has 2 extra bishops (all nonpawns are bishops)
            return( abs(  non_pawn_count<WHITE>()
                        - non_pawn_count<BLACK>()) < 2);

        }

        // Draw with 1-2 knights vs 1 bishop (there is at least 1 bishop, and at last 1 knight)
        else if (BB::count_bit(typePiecesBB[BISHOP]) == 1)
        {
            if (BB::count_bit(occupancy_cp<WHITE, BISHOP>()) == 1)
                return non_pawn_count<WHITE>() == 1 && non_pawn_count<BLACK>() <= 2;
            else
                return non_pawn_count<BLACK>() == 1 && non_pawn_count<WHITE>() <= 2;
        }
    }

    // Draw with 1 rook + up to 1 minor each
    else if (   BB::count_bit(occupancy_cp<WHITE, ROOK>()) == 1
             && BB::count_bit(occupancy_cp<BLACK, ROOK>()) == 1)
    {
        return    non_pawn_count<WHITE>() <= 2
               && non_pawn_count<BLACK>() <= 2;
    }

    // Draw with 1 rook vs 1-2 minors
    else if ( BB::count_bit(typePiecesBB[ROOK]) == 1)
    {
        if (BB::count_bit(occupancy_cp<WHITE, ROOK>()) == 1)
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

// Calculate scale factor to lower overall eval based on various features
int Board::scale_factor(const Score eval)
{
    // Scale down eval for opposite-colored bishops endgames
    if (   !typePiecesBB[QUEEN]
        && !typePiecesBB[ROOK]
        && !typePiecesBB[KNIGHT]
        && non_pawn_count<WHITE>() == 1
        && non_pawn_count<BLACK>() == 1
        && (BB::single(typePiecesBB[BISHOP] & DarkSquares)))
        return 64;

    // Scale down eval the fewer pawns the stronger side has
    //    Color strong = eval > 0 ? WHITE : BLACK;
    int strongPawnCount = eval > 0 ? BB::count_bit(occupancy_cp<WHITE, PAWN>())
                                   : BB::count_bit(occupancy_cp<BLACK, PAWN>());
    int x = 8 - strongPawnCount;
    return 128 - x * x;

}

//=============================================================
//! \brief  Initialisation des informations de l'évaluation
//-------------------------------------------------------------
void Board::init_eval_info(EvalInfo& ei)
{
    // Initialisation des bitboards
    constexpr Direction Down[2] = {SOUTH, NORTH};                  // direction "bas", relativement à la couleur
    constexpr Direction Up[2]   = {NORTH, SOUTH};                  // direction "haut", relativement à la couleur
    constexpr Bitboard  RelRanK2BB[2] = {                          // La rangée 2, relativement à la couleur
                                        RankMask8[SQ::relative_rank8<WHITE>(RANK_2)],
                                        RankMask8[SQ::relative_rank8<BLACK>(RANK_2)],
                                        };

    ei.phase24 = 0;
    ei.attackCount[WHITE] = 0;
    ei.attackCount[BLACK] = 0;
    ei.attackPower[WHITE] = 0;
    ei.attackPower[BLACK] = 0;

    ei.occupied     = occupancy_all();                                   // toutes les pièces (Blanches + Noires)
    ei.pawns[WHITE] = occupancy_cp<WHITE, PAWN>();
    ei.pawns[BLACK] = occupancy_cp<BLACK, PAWN>();

    ei.knights[WHITE] = occupancy_cp<WHITE, KNIGHT>();
    ei.knights[BLACK] = occupancy_cp<BLACK, KNIGHT>();

    ei.bishops[WHITE] = occupancy_cp<WHITE, BISHOP>();
    ei.bishops[BLACK] = occupancy_cp<BLACK, BISHOP>();

    ei.rooks[WHITE] = occupancy_cp<WHITE, ROOK>();
    ei.rooks[BLACK] = occupancy_cp<BLACK, ROOK>();

    ei.queens[WHITE] = occupancy_cp<WHITE, QUEEN>();
    ei.queens[BLACK] = occupancy_cp<BLACK, QUEEN>();

    // La zone de mobilité est définie ainsi (idée de Weiss) : toute case
    //  > ni attaquée par un pion adverse
    //  > ni occupée par un pion ami sur sa case de départ
    //  > ni occupée par un pion ami bloqué

    const Bitboard b[2] = {
        (RelRanK2BB[WHITE] | BB::shift<Down[WHITE]>(ei.occupied)) & ei.pawns[WHITE],
        (RelRanK2BB[BLACK] | BB::shift<Down[BLACK]>(ei.occupied)) & ei.pawns[BLACK]
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
    ei.enemyKingZone[WHITE] = auxi[WHITE] | BB::shift<Up[WHITE]>(auxi[WHITE]);
    ei.enemyKingZone[BLACK] = auxi[BLACK] | BB::shift<Up[BLACK]>(auxi[BLACK]);

    // Attaques des pions
    ei.pawnAttacks[WHITE] = all_pawn_attacks<WHITE>(ei.pawns[WHITE]);
    ei.pawnAttacks[BLACK] = all_pawn_attacks<BLACK>(ei.pawns[BLACK]);

    ei.attackPower[WHITE] = -30;
    ei.attackCount[WHITE] = 0;
    ei.attackPower[BLACK] = -30;
    ei.attackCount[BLACK] = 0;

    // Clear passed pawns, filled in during pawn eval
    ei.passedPawns = 0;

}

