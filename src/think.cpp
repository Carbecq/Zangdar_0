#include "MovePicker.h"
#include "OrderingInfo.h"
#include "Search.h"
#include "ThreadPool.h"
#include "Timer.h"
#include "TranspositionTable.h"
#include <iostream>

extern ThreadPool threadPool;


//======================================================
//! \brief  Lancement d'une recherche
//!
//! itérative deepening
//! alpha-beta
//!
//------------------------------------------------------
template<Color C>
void Search::think(int threadID)
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "Search::think (thread=%d)", threadID);
    printlog(message);
#endif

    ThreadData* td = &threadPool.threads[threadID];

    my_timer.start();
    my_timer.setup(C);

    int max_depth = my_timer.getSearchDepth();
    int score     = 0;
    int alpha     = -INFINITE;
    int beta      = INFINITE;
    int delta     = 0;
    int sdepth;

    bool do_NULL  = true;
    PVariation pv;
    pv.length = 0;

    int  ply = 0;

    // iterative deepening
    for (int depth = 1; depth <= max_depth; ++depth)
    {
        //---------------------------------------------------------------------
        //  ASPIRATION WINDOW
        //---------------------------------------------------------------------
        if (depth >= 6)
        {
            alpha = std::max(td->best_score - ASPIRATION_WINDOW, -INFINITE);
            beta  = std::min(td->best_score + ASPIRATION_WINDOW, INFINITE);
            delta = ASPIRATION_WINDOW;
            sdepth = depth;

            while (true)
            {
                score = alpha_beta<C>(my_board, ply, alpha, beta, std::max(1, sdepth), do_NULL, pv, td);

                if (stopped)
                    break;

                if (score <= alpha) // Fail Low
                {
                    beta  = (alpha + beta) / 2;
                    alpha = std::max(score - delta, -INFINITE); // alpha/score-delta
                    sdepth = depth;
                }
                else if(score >= beta)  // Fail High
                {
                    beta  = std::min(score + delta, INFINITE);   // beta/score+delta
                    // idée de Berserk
                    if (abs(score) < TBWIN_IN_X)
                        sdepth--;
                }
                else
                {
                    break;
                }

                delta += delta/4;
            }
        }
        else
        {
            score = alpha_beta<C>(my_board, ply, -INFINITE, INFINITE, depth, do_NULL, pv, td);
        }

        if (stopped)
            break;

        // L'itération s'est terminée sans problème
        // On peut mettre à jour les infos UCI
        if (threadID == 0)
        {
            td->best_depth = depth;
            td->best_move  = pv.line[0];
            td->best_score = score;
        }

        U64 elapsed     = 0; // durée en millisecondes
        bool shouldStop = my_timer.finishOnThisDepth(elapsed);

        if (logUci && threadID == 0)
            show_uci_result(td, elapsed, pv);

        // est-ce qu'on a le temps pour une nouvelle itération ?
        if (shouldStop == true)
            break;
    }

    if (threadID == 0)
    {
        if (logUci)
        {
            show_uci_best(td);
            my_timer.show_time();
        }

        threadPool.main_thread_stopped(); // arrêt des autres threads
        threadPool.wait(1);               // attente de ces threads
    }
    //  Transtable.stats();
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
template<Color C>
int Search::alpha_beta(Board &board, int ply, int alpha, int beta, int depth, bool do_NULL, PVariation& pv, ThreadData* td)
{
    assert(board.valid<C>());
    assert(beta > alpha);

    td->nodes++;

    PVariation new_pv;
    new_pv.length = 0;
    pv.length     = 0;

    // On a atteint la limite en profondeur de recherche ?
    if (ply >= MAX_PLY - 1)
        return board.evaluate<true>();

    // On a atteint la limite de taille de la partie ?
    if (board.game_clock >= MAX_HIST - 1)
        return board.evaluate<true>();

    //  Time-out
    if (stopped || check_limits(td))
    {
        stopped = true;
        return 0;
    }

    //  Caractéristiques de la position
    bool isRoot   = (ply == 0);
    bool isPVNode = ((beta - alpha) != 1); // We are in a PV-node if we aren't in a null window.
    bool inCheck  = board.is_in_check<C>();
    int  best_score = -INFINITE;        // initially assume the worst case
    MOVE best_move  = Move::MOVE_NONE;  // meilleur coup local
    int  max_score  = INFINITE;         // best possible

    //  Check Extension
    if (inCheck == true && depth + 1 < MAX_PLY)
        depth++;

    /* On a atteint la fin de la recherche
        Certains codes n'appellent la quiescence que si on n'est pas en échec
        ceci amène à une explosion des coups recherchés */
    if (depth <= 0)
    {
        td->nodes--;
        return (quiescence<C>(board, ply, alpha, beta, td));
    }

    if (!isRoot)
    {
        //  Position nulle ?
        if (board.is_draw<C>())
            return CONTEMPT;

        // Mate distance pruning
        alpha = std::max(alpha, -MATE + ply);
        beta  = std::min(beta,   MATE - ply - 1);
        if (alpha >= beta)
            return alpha;
    }

    //  Recherche de la position actuelle dans la table de transposition
    int  tt_score;
    MOVE tt_move  = Move::MOVE_NONE;
    int  tt_flag;
    int  tt_depth;
    bool tt_hit   = Transtable.probe(board.hash, ply, tt_move, tt_score, tt_flag, tt_depth);
    bool tt_tactical = false;

    if (tt_hit)
    {
        tt_tactical = Move::is_tactical(tt_move);
        if (tt_depth >= depth && (depth == 0 || !isPVNode))
        {
            if (    tt_flag == HASH_EXACT
                || (tt_flag == HASH_BETA  && tt_score >= beta)  // lower
                || (tt_flag == HASH_ALPHA && tt_score <= alpha)) // upper
            {
                return tt_score;
            }
        }
    }

    // Probe the Syzygy Tablebases
    int tbScore, tbBound;
    if (UseSyzygy && board.probe_wdl(tbScore, tbBound, ply) == true)
    {
        //   thread->tbhits++;

        // Check to see if the WDL value would cause a cutoff
        if (    tbBound == HASH_EXACT
            || (tbBound == HASH_BETA && tbScore >= beta)    // lower
            || (tbBound == HASH_ALPHA && tbScore <= alpha)) // upper
        {
            Transtable.store(board.hash, Move::MOVE_NONE, tbScore, tbBound, depth, ply);
            return tbScore;
        }

        // Limit the score of this node based on the tb result
        if (isPVNode)
        {
            // Never score something worse than the known Syzygy value
            if (tbBound == HASH_BETA /*BOUND_LOWER*/)
            {
                best_score = tbScore;
                alpha = std::max(alpha, tbScore);
            }

            // Never score something better than the known Syzygy value
            if (tbBound == HASH_ALPHA)
            {
                max_score = tbScore;
            }
        }
    }


    //  Evaluation Statique
    int static_eval;

    if (inCheck)
    {
        static_eval = NOSCORE;
    }
    else
    {
        static_eval = board.evaluate<true>();
        statEval[ply] = static_eval;
    }

    /*  Avons-nous amélioré la position ?
        Si on ne s'est pas amélioré dans cette ligne, on va pouvoir couper un peu plus */
    bool improving = false;
    if (ply > 2)
        improving = !inCheck && (static_eval > statEval[ply - 2]);

    //  Controle si on va pouvoir utiliser des techniques de coupe pre-move
    int  score;

    if (!inCheck && !isRoot)
    {
        //---------------------------------------------------------------------
        //  RAZORING
        //---------------------------------------------------------------------
        if (!isPVNode && depth <= 3 && (static_eval + 200 * depth) <= alpha)
        {
            score = quiescence<C>(board, ply, alpha, beta, td);
            if (score <= alpha)
            {
                td->nodes--;
                return score;
            }
        }

        //---------------------------------------------------------------------
        //  STATIC NULL MOVE PRUNING ou aussi REVERSE FUTILITY PRUNING
        //---------------------------------------------------------------------
        if (   !isPVNode
            && depth <= 6
            && abs(beta) < MATE_IN_X
            && board.getNonPawnMaterial<C>())
        {
            int eval_margin = 70 * depth;

            if (static_eval - eval_margin >= beta)
            {
                return static_eval - eval_margin; // Fail Soft
            }
        }

        //---------------------------------------------------------------------
        //  NULL MOVE PRUNING
        //---------------------------------------------------------------------
        if (   do_NULL
            && ply
            && static_eval >= beta
            && depth > NULL_MOVE_R + 1      //TODO : Vice ajoute un test sur ply
            && board.major_pieces<C>() > 0) //TODO voir le test sur les pièces
        {
            int R = NULL_MOVE_R;
            if (depth > 6)
                R++;

            board.make_nullmove<C>();
            score = -alpha_beta<~C>(board, ply + 1, -beta, -beta + 1, depth - 1 - R, false, new_pv, td);
            board.undo_nullmove<C>();

            if (stopped)
                return 0;

            // (Stockfish) : on ne retourne pas un score proche du mat
            //               car ce score ne serait pas prouvé
            if (score >= beta)
            {
                if (score > TBWIN_IN_X)
                    score = beta;
                return score;
            }
        }

    } // end Pruning

    //---------------------------------------------------------------------
    // Internal Iterative Deepening.
    //---------------------------------------------------------------------
    if (    isPVNode
        &&  tt_move == Move::MOVE_NONE
        &&  depth >= 3)
    {
        alpha_beta<C>(board, ply, -beta, -alpha, depth - 2, true, new_pv, td);

        // Probe for the newly found move, and update ttMove / ttTactical
        if (Transtable.probe(board.hash, ply, tt_move, tt_score, tt_flag, tt_depth))
            tt_tactical = Move::is_tactical(tt_move);
    }

    //====================================================================================
    //  Génération des coups
    //------------------------------------------------------------------------------------
    MoveList move_list;
    board.legal_moves<C>(move_list);

    MovePicker movePicker(ply, tt_move, &my_orderingInfo, &board, &move_list);

    int  played     = 0;
    MOVE move;
    const int old_alpha = alpha;
    int quiets = 0;

    // Boucle sur tous les coups
    while (movePicker.hasNext())
    {
        move = movePicker.getNext();

        bool isQuiet = !Move::is_tactical(move);

        // Affichage du coup courant
#ifdef ACC
        if (ply==0 && isPVNode && !td->index && my_timer.elapsedTime() > CurrmoveTimerMS)
            show_uci_current(move, legalMoves, depth);
#endif

        //-------------------------------------------------
        //  Late Move Pruning / Move Count Based Pruning
        //-------------------------------------------------
#ifdef LMP
        if (   !isPVNode
            && !inCheck
            && quiets > (3 + 2 * depth * depth) / (2 - improving) )
            continue;
#endif

        // execute current move
        board.make_move<C>(move);

        // Update counter of moves actually played
        played++;
        quiets += isQuiet;

        //------------------------------------------------------------------------------------
        //  LATE MOVE REDUCTION
        //------------------------------------------------------------------------------------
        int R;
        if (    played >= 4
            &&  depth >= 3
            && !inCheck
            &&  isQuiet
            )
        {
            R = 2;
            R -= isRoot;
            R += (played - 4) / 8;
            R += 2*!isPVNode;
            R += tt_tactical && best_move == tt_move;

            R = R >= 1 ? R : 1;
        }
        else
        {
            R = 1;
        }

        // Search the move with a possibly reduced depth, on a full or null window
        score =  (played == 1 || !isPVNode)
                    ? -alpha_beta<~C>(board, ply + 1, -beta, -alpha, depth-R, true, new_pv, td)
                    : -alpha_beta<~C>(board, ply + 1, -alpha-1, -alpha, depth-R, true, new_pv, td);

        // If the search beat alpha, we may need to research, in the event that
        // the previous search was not the full window, or was a reduced depth
        score =  (score > alpha && (R != 1 || (played != 1 && isPVNode)))
                    ? -alpha_beta<~C>(board, ply + 1, -beta, -alpha, depth-1, true, new_pv, td)
                    :  score;


        // retract current move
        board.undo_move<C>();

        //  Time-out
        if (stopped)
            return 0;

        // On a trouvé un nouveau meilleur coup
        if (score > best_score)
        {
            best_score = score; // le meilleur que l'on ait trouvé, pas forcément intéressant
            best_move  = move;

            // If score beats alpha we update alpha
            if (score > alpha)
            {
                alpha = score;

                // update the PV
                if (td->index == 0)
                    update_pv(pv, new_pv, move);

                // Update search history
                if (isQuiet)
                    my_orderingInfo.incrementHistory(C, Move::piece(move), Move::dest(move), depth);

                // If score beats beta we have a cutoff
                if (score >= beta)
                {
                    // non, ce coup est trop bon pour l'adversaire
                    // Update Killers
                    if (isQuiet)
                    {
                        my_orderingInfo.updateKillers(ply, move);
                        //TODO update history ?
                    }

                    Transtable.store(board.hash, move, score, HASH_BETA, depth, ply);
                    return score;
                }


            } // score > alpha
        }     // meilleur coup
    }         // boucle sur les coups

    // est-on mat ou pat ?
    if (played == 0)
    {
        // On est en échec, et on n'a aucun coup : on est MAT
        // On n'est pas en échec, et on n'a aucun coup : on est PAT
        return inCheck ? -MATE + ply : 0;
    }

    // don't let our score inflate too high (tb)
    best_score = std::min(best_score, max_score);

    if (!stopped)
    {
        //  si on est ici, c'est que l'on a trouvé au moins 1 coup
        //  et de plus : score < beta
        //  si score >  alpha    : c'est un bon coup : HASH_EXACT
        //  si score <= alpha    : c'est un coup qui n'améliore pas alpha : HASH_ALPHA
        int flag = (alpha != old_alpha) ? HASH_EXACT : HASH_ALPHA;
        Transtable.store(board.hash, best_move, best_score, flag, depth, ply);
    }

    return best_score;
}

template void Search::think<WHITE>(int id);
template void Search::think<BLACK>(int id);

template int Search::alpha_beta<WHITE>(Board &board, int ply, int alpha, int beta, int depth, bool do_NULL, PVariation& pv, ThreadData* td);
template int Search::alpha_beta<BLACK>(Board &board, int ply, int alpha, int beta, int depth, bool do_NULL, PVariation& pv, ThreadData* td);
