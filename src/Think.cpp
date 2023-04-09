#include <iostream>
#include "Search.h"
#include "Timer.h"


//======================================================
//! \brief  Lancement d'une recherche
//!
//! itérative deepening
//! alpha-beta
//!
//------------------------------------------------------
template <Color C>
void Search::think(Board* m_board, Timer* m_timer)
{
    assert(m_board);
    assert(m_timer);

    board = m_board;
    timer = m_timer;

    new_search();
    timer->start();
    timer->setup(C, board->game_clock);
    transtable.update_age();

    int max_depth = timer->getSearchDepth();
    int score;
    int alpha = -INF;
    int beta  = INF;
    bool do_NULL = true;
    MOVE pv[MAX_PLY] = {0};
    int  ply = 0;
    best = 0;

    // iterative deepening
    for(int depth = 1; depth <= max_depth; ++depth )
    {
        score = alpha_beta<C>(ply, alpha, beta, depth, do_NULL, pv);

        if (stopped)
            break;

        // L'itération s'est terminée sans problème
        // On peut mettre à jour les infos UCI
        best            = pv[0];
        U64  elapsed    = 0;    // durée en millisecondes
        bool shouldStop = timer->finishOnThisDepth(elapsed);

        if (logUci)
            show_uci_result(depth,  score, elapsed, pv);

        // un score supérieur à MAX_EVAL signifie que l'on a
        // trouvé un mat
        if (abs(score) > MAX_EVAL)
            break;

        // est-ce qu'on a le temps pour une nouvelle itération ?
        if (shouldStop == true)
            break;
    }

    if (logUci)
        show_uci_best(best);

    if (logSearch)
        timer->show_time();

    //       transtable.stats();
}

//=====================================================
//! \brief  Recherche du meilleur coup
//!
//!     Méthode Apha-Beta - Fail Soft
//!
//! \param[in]  ply     profondeur actuelle de recherche
//! \param[in]  alpha   borne inférieure : on est garanti d'avoir au moins alpha
//! \param[in]  beta    borne supérieure : meilleur coup pour l'adversaire
//! \param[in]  depth   profondeur maximum de recherche
//! \param[in]  doNull  on va effectuer un Null Move
//! \param[out] pv      tableau de stockage de la Variation Principale
//!
//! \return Valeur du score
//-----------------------------------------------------
template <Color C>
int Search::alpha_beta(int ply, int alpha, int beta, int depth, bool do_NULL, MOVE* pv )
{
    assert(board->valid<C>());
    assert(beta > alpha);

    MOVE move;

    nodes++;

    if (ply)
        *pv = 0;

    // On a atteint la limite en profondeur de recherche ?
    if(ply >= MAX_PLY - 1)
        return board->evaluate();

    // On a atteint la limite de taille de la partie ?
    if (board->game_clock >= MAX_HIST - 1)
        return board->evaluate();

    //  Time-out
    if (stopped || check_limits()) {
        stopped = true;
        return 0;
    }

    bool isRoot   = (ply == 0);
    bool isPVNode = ((beta-alpha) != 1);
    bool inCheck  = board->is_in_check<C>();

    MOVE new_pv[MAX_PLY] = {0};

    //====================================================================================
    //  CHECK EXTENSION
    //------------------------------------------------------------------------------------
    if (inCheck == true)
        depth++;

    //====================================================================================
    // On a atteint la fin de la recherche
    // Certains codes n'appellent la quiescence que si on n'est pas en échec
    // ceci amène à une explosion des coups recherchés
    //------------------------------------------------------------------------------------
    if (depth <= 0)
    {
        nodes--;
        return(quiescence<C>(ply, alpha, beta, pv));
    }

    //====================================================================================
    //  Position nulle ?
    //------------------------------------------------------------------------------------
    if(!isRoot && board->is_draw<C>())
        return 0;

    //====================================================================================
    //  Recherche de la position actuelle dans la hashtable
    //------------------------------------------------------------------------------------
    int  score   = -INF;
    MOVE tt_move = 0;
    int  flag    = HASH_NONE;
    bool ttm     = transtable.probe(board->hash, tt_move, score, flag, alpha, beta, depth, ply);

    if (score != INVALID && !isRoot && ttm == true)
    {
        // mettre à jour killer si beta ?
        return score;
    }

    //====================================================================================
    //  Evaluation Statique
    //------------------------------------------------------------------------------------
    int static_eval;

    if (inCheck) {
        static_eval = INF; // ??? _sStack.AddEval(NOSCORE);
    }else {
        static_eval   = board->evaluate();
        statEval[ply] = static_eval;
    }

    //====================================================================================
    //  Avons-nous amélioré la position ?
    //  Si on ne s'est pas amélioré dans cette ligne, on va pouvoir couper un peu plus
    //-------------------------------------------------------------------------------------
    bool improving = false;
    if (ply > 2)
        improving = !inCheck && static_eval > statEval[ply - 2];

    //====================================================================================
    //  Controle si on va pouvoir utiliser des techniques de coupe pre-move
    //------------------------------------------------------------------------------------
    bool isPrune = !isPVNode && !inCheck /*&& !sing */;     // proving singularity (Drofa)

    //====================================================================================
    // STATIC NULL MOVE PRUNING ou aussi REVERSE FUTILITY PRUNING
    //------------------------------------------------------------------------------------
    if (isPrune && depth < 6 && abs(beta) < MAX_EVAL)
    {
        int eval_margin = 150 * depth + 100 * improving;    // Drofa
        if (static_eval - eval_margin >= beta) {
            return static_eval - eval_margin;
        }
    }

    //====================================================================================
    // NULL MOVE PRUNING
    //------------------------------------------------------------------------------------
    if( do_NULL && isPrune && static_eval >= beta && depth > NULL_MOVE_R + 1
            && board->major_pieces<C>() > 0)
    {
        int R = NULL_MOVE_R;
        if (depth > 6)  //TODO à vérifier
            R++;

        board->make_nullmove<C>();
        score = -alpha_beta<~C>(ply+1, -beta, -beta + 1, depth - 1 - R, false, new_pv);
        board->undo_nullmove<C>();

        if (stopped)
            return 0;

        // (Stockfish) : on ne retourne pas un score proche du mat
        //               car ce score ne serait pas prouvé
        if (score >= beta && abs(score) < MAX_EVAL )
        {
            //TODO ?           transtable.store(board->hash, 0, score, LOWER, depth, ply);
            return score;
        }
    }

    //====================================================================================
    //  Génération des coups
    //------------------------------------------------------------------------------------
    MoveList move_list;
    board->legal_moves<C>(move_list);
    order_moves(ply, move_list, tt_move);

    int  legalMoves = 0;
    int  ttFlag = HASH_ALPHA;
    int  best_score  = -INF;   // meilleur coup local
    MOVE best_move = 0;

    // Boucle sur tous les coups
    for (int index=0; index<move_list.count; index++)
    {
        PickNextMove(move_list, index);
        move = move_list.moves[index];
        legalMoves++;

        // execute current move
        board->make_move<C>(move);


         if (best_score == -INF)     // ou : legal_moves==1
        {
            score = -alpha_beta<~C>(ply+1, -beta, -alpha, depth-1, true, new_pv);
        }
        else
        {
            // On peut essayer la PVS (Principal Variation Search)
            score = -alpha_beta<~C>(ply+1, -alpha-1, -alpha, depth-1, true, new_pv);

            // Est-ce que ça a marché ?
            if ( !stopped && score > alpha && score < beta )
            {
                // Raté, il faut refaire une recherche complète
                score = -alpha_beta<~C>(ply+1, -beta, -alpha, depth-1, true, new_pv);
            }
        }

        // retract current move
        board->undo_move<C>();

        //  Time-out
        if (stopped)
            return 0;

        if(score >= beta)
        {
            // non, ce coup est trop bon pour l'adversaire
            // Killer Heuristic
            // On stocke 2 Killers (ce qui semble suffisant).
            // Lors de la prochaine génération de coups, si on trouve un coup
            // qui est un Killer, on va modifier son score
            //  ->add_quiet_move
            // Puis quand dans la recherche alpha-beta, lorsqu'on cherche
            // le meilleur coup (PickNextMove), les 2 Killers Move
            // vont remonter vers la tête.

            if (Move::is_capturing(move) == false)
            {
                setKillers(move, ply);
                //TODO update history ?
            }
            transtable.store(board->hash, move, score, HASH_BETA, depth, ply);
            return score;
        }

        // On a trouvé un bon coup
        if(score > best_score)
        {
            best_score = score;     // le mieux que l'on ait trouvé, pas forcément intéressant

            if(score > alpha)
            {
                alpha  = score;

                // alpha cutoff
                // https://www.chessprogramming.org/History_Heuristic
                if (Move::is_capturing(move) == false)
                {
                    searchHistory[C][Move::piece(move)][Move::dest(move)] += depth;
                }

                /* update the PV */
                update_pv(pv, new_pv, move);
                //                if (ply == 0)
                //                    show_uci_result(depth, best_score, 1, pv);

            } // score > alpha
        } // meilleur coup
    } // boucle sur les coups

    // est-on mat ou pat ?
    if (best_score == -INF)
    {
        if (inCheck)
        {
            // On est en échec, et on n'a aucun coup : on est MAT
            //     std::cout <<  "on est echec" << std::endl;

            return -MATE + ply;
        }
        else
        {
            // On n'est pas en échec, et on n'a aucun coup : on est PAT
            return 0;
        }
    }

    // faut-il écrire le coup trouvé dans la hashtable, même si alpha n'est pas amélioré ?
    // On écrit le meilleur coup dans la hashtable
    if (!stopped)
    {
        if (*pv)
            transtable.store(board->hash, *pv, best_score, HASH_EXACT, depth, ply);
        else
            transtable.store(board->hash, 0, best_score, HASH_ALPHA, depth, ply);
    }

    return best_score;
}


//=============================================================
//! \brief  Recherche le meilleur coup à partir de moveNum
//-------------------------------------------------------------
void Search::PickNextMove(MoveList& move_list, int moveNum)
{
    int score     = -INF;
    int bestNum   = moveNum;

    for (int index = moveNum; index < move_list.count; index++)
    {
        if (move_list.values[index] > score)
        {
            score   = move_list.values[index];
            bestNum = index;
        }
    }

    if (moveNum != bestNum)
    {
        move_list.swap(moveNum, bestNum);
    }
}
//=============================================================
//! \brief  Met à jour les Killers
//-------------------------------------------------------------
void Search::setKillers(U32 move, int ply)
{
    if (searchKillers[0][ply] != move)
    {
        searchKillers[1][ply] = searchKillers[0][ply];
        searchKillers[0][ply] = move;
    }
}

template void Search::think<WHITE>(Board* m_board, Timer* m_timer);
template void Search::think<BLACK>(Board* m_board, Timer* m_timer);

template int Search::alpha_beta<WHITE>(int ply, int alpha, int beta, int depth, bool do_NULL, MOVE* pv );
template int Search::alpha_beta<BLACK>(int ply, int alpha, int beta, int depth, bool do_NULL, MOVE* pv );

