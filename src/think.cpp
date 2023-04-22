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
    int alpha     = -MATE;
    int beta      = MATE;
    int delta     = 0;
    int sdepth;
  //  int window = 23;

    bool do_NULL  = true;
    MOVE pv[MAX_PLY] = {0};
    int ply = 0;

    if (threadID == 0)
        Transtable.update_age();

    // iterative deepening
    for (int depth = 1; depth <= max_depth; ++depth)
    {
        //====================================================================================
        //  ASPIRATION WINDOW
        //------------------------------------------------------------------------------------
        if (depth >= 6)
        {
            alpha = std::max(td->best_score - WINDOW, -MATE);
            beta  = std::min(td->best_score + WINDOW, MATE);
            delta = WINDOW;
            sdepth = depth;

            while (true)
            {
                score = alpha_beta<C>(my_board, ply, alpha, beta, std::max(1, sdepth), do_NULL, pv, td);

                if (stopped)
                    break;

                if (score <= alpha)
                {
                    // Fail Low
                    beta  = (alpha + beta) / 2;
                    alpha = std::max(score - delta, -MATE); // alpha/score-delta
                    sdepth = depth;
                }
                else if(score >= beta)
                {
                    // Search failed high, adjust window and reduce depth
                    beta  = std::min(score + delta, MATE);   // beta/score+delta
                    if (abs(score) < TB_WIN_BOUND)
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
            score = alpha_beta<C>(my_board, ply, -MATE, MATE, depth, do_NULL, pv, td);
        }

        if (stopped)
            break;

        // L'itération s'est terminée sans problème
        // On peut mettre à jour les infos UCI
        td->best_depth = depth;
        td->best_move  = pv[0];
        td->best_score = score;

        U64 elapsed     = 0; // durée en millisecondes
        bool shouldStop = my_timer.finishOnThisDepth(elapsed);

        if (logUci && threadID == 0)
        {
            show_uci_result(td, elapsed, pv);
            //   printf("fh=%d  fl=%d fok=%d \n", fh, fl, fok);
        }

        // un score supérieur à MAX_EVAL signifie que l'on a
        // trouvé un mat
        if (abs(score) > MAX_EVAL)
            break;

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
    //       Transtable.stats();
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
int Search::alpha_beta(Board &board, int ply, int alpha, int beta, int depth, bool do_NULL, MOVE *pv, ThreadData* td)
{
    assert(board.valid<C>());
    assert(beta > alpha);

    td->nodes++;

    if (ply)
        *pv = 0;

    // On a atteint la limite en profondeur de recherche ?
    if (ply >= MAX_PLY - 1)
        return board.evaluate();

    // On a atteint la limite de taille de la partie ?
    if (board.game_clock >= MAX_HIST - 1)
        return board.evaluate();

    //  Time-out
    if (stopped || check_limits(td))
    {
        stopped = true;
        return 0;
    }

    bool isRoot = (ply == 0);
    bool isPVNode = ((beta - alpha) != 1); // We are in a PV-node if we aren't in a null window.
    bool inCheck = board.is_in_check<C>();
    //    bool canFutility = false;

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
        td->nodes--;
        return (quiescence<C>(board, ply, alpha, beta, pv, td));
    }

    //====================================================================================
    //  Position nulle ?
    //------------------------------------------------------------------------------------
    if (!isRoot && board.is_draw<C>())
        return CONTEMPT;

    //====================================================================================
    //  Recherche de la position actuelle dans la hashtable
    //------------------------------------------------------------------------------------
    int score = -INF;
    MOVE tt_move = 0;
    int flag = HASH_NONE;
    bool ttm = Transtable.probe(board.hash, tt_move, score, flag, alpha, beta, depth, ply);

    if (score != INVALID && !isRoot && ttm == true)
    {
        // mettre à jour killer si beta ?
        return score;
    }

    //====================================================================================
    //  Evaluation Statique
    //------------------------------------------------------------------------------------
    int static_eval;

    if (inCheck)
    {
        static_eval = INF; // ??? _sStack.AddEval(NOSCORE);
    }
    else
    {
        static_eval = board.evaluate();
        statEval[ply] = static_eval;
    }

    //====================================================================================
    //  Avons-nous amélioré la position ?
    //  Si on ne s'est pas amélioré dans cette ligne, on va pouvoir couper un peu plus
    //-------------------------------------------------------------------------------------
    //    bool improving = false;
    //    if (ply > 2)
    //        improving = !inCheck && (static_eval > statEval[ply - 2]);

    //====================================================================================
    //  Controle si on va pouvoir utiliser des techniques de coupe pre-move
    //------------------------------------------------------------------------------------
    bool canPrune = !isPVNode && !inCheck /*&& !sing */; // proving singularity (Drofa)

    if (canPrune)
    {
        //        static constexpr int REVF_MOVE_CONST = 150;

        //====================================================================================
        //  STATIC NULL MOVE PRUNING ou aussi REVERSE FUTILITY PRUNING
        //  If our static evaluation beats beta by the futility margin,
        //  we can most likely just return beta.
        //  (Implémentation de Laser)
        //------------------------------------------------------------------------------------
        if (depth <= 6 && abs(beta) < MAX_EVAL && board.getNonPawnMaterial<C>())
        {
            // int eval_margin = 175 * depth - ((improving) ? 75 : 0);       // Loki  non=175 oui=100 = 175 * depth - 75 * improving
            // int eval_margin = 150 * depth - 100 * improving;              // Drofa  non=150 - oui=50
            // int eval_margin = 120 * depth;                                // Blunder 7.1
            // int eval_margin = 85 * depth;                                   // Blunder 855
            int eval_margin = 70 * depth; // OK ici              // Laser
            // int eval_margin = 50 * (depth - improving);                   // Berserk

            if (static_eval - eval_margin >= beta)
            {
                return static_eval - eval_margin; // Fail Soft
            }
        }

        //====================================================================================
        //  RAZORING
        //------------------------------------------------------------------------------------
        if (depth <= 3 && (static_eval + 200 * depth) <= alpha)
        {
            // Berserk
            score = quiescence<C>(board, ply, alpha, beta, pv, td);
            if (score <= alpha)
            {
                td->nodes--;
                return score;
            }
        }

        //====================================================================================
        // NULL MOVE PRUNING
        //------------------------------------------------------------------------------------
        if (do_NULL && ply && static_eval >= beta
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
            if (score >= beta && abs(score) < MAX_EVAL)
            {
                //TODO ?           Transtable.store(board.hash, 0, score, LOWER, depth, ply);
                return score;
            }
        }

        //====================================================================================
        //   FUTILITY PRUNING
        //------------------------------------------------------------------------------------
        //        if (depth <= 8 && abs(alpha) < MAX_EVAL
        //            && (static_eval + FutilityMargin[depth]) <= alpha) {
        //            canFutility = true;
        //        }

        //====================================================================================
        //  ENHANCED FUTILITY PRUNING
        //  If our position seems so bad that it can't possibly raise alpha,
        //  we can set a futility_pruning flag
        //	and skip tactically boring moves from the search
        //------------------------------------------------------------------------------------
        //    if (depth <= 3 && isPrune && abs(alpha) < MAX_EVAL
        //        && (static_eval + futility_margin(depth, improving)) <= alpha) {

        //        doFutility = true;
        //    }

    } // end Pruning

    //====================================================================================
    //  Génération des coups
    //------------------------------------------------------------------------------------
    MoveList move_list;
    board.legal_moves<C>(move_list);

    MovePicker movePicker(ply, tt_move, &my_orderingInfo, &board, &move_list);

    int legalMoves = 0;
    int best_score = -INF; // meilleur coup local
    MOVE move;

    // Boucle sur tous les coups
    while (movePicker.hasNext())
    {
        move = movePicker.getNext();

        legalMoves++;

        // execute current move
        board.make_move<C>(move);

        //   bool doCheck    = board.is_in_check<~C>();
        bool isTactical = inCheck || /*doCheck || */ Move::is_capturing(move)
                          || Move::is_promoting(move);

//====================================================================================
//  FUTILITY PRUNING
//  If we are allowed to use futility pruning,
//  and this move is not tactically significant, prune it.
//  We just need to make sure that at least one legal move
//  has been searched since we'd risk getting false mate scores else.
//------------------------------------------------------------------------------------

//  Légère perte d'ELO

//        if (canFutility && !isTactical && legalMoves > 1) {
//            board.undo_move<C>();
//            continue;
//        }

//        if (canFutility && legalMoves > 1)
//        {
//            bool tactical = board.is_in_check<~C>() || Move::is_capturing(move) || Move::is_promoting(move);
//            if (!tactical)
//            {
//                board.undo_move<C>();
//                continue;
//            }
//        }

//====================================================================================
//  LATE MOVE REDUCTION
// With good move ordering, later moves are less likely to increase
// alpha, so we search them to a shallower depth hoping for a quick
// fail-low.
//------------------------------------------------------------------------------------
#ifdef LMR
        if (legalMoves == 1)
        {
            // first move, use full depth search
            score = -alpha_beta<~C>(board, ply + 1, -beta, -alpha, depth - 1, true, new_pv, td);
        }
        else
        {
            // late move reduction (LMR)
            //   int reduction = 0;

            if (legalMoves >= LMRLegalMovesLimit - 1 && !isPVNode && depth >= LMRDepthLimit
                && !isTactical)
            {
                //   reduction = 1;

                // search current move with reduced depth:
                score = -alpha_beta<~C>(board, ply + 1, -alpha - 1, -alpha, depth - 2, true, new_pv, td);
            }
            else
            {
                // Hack to ensure that full-depth search is done
                score = alpha + 1;
            }

            if (score > alpha)
            {
                score =
                    -alpha_beta<~C>(board, ply + 1, -(alpha + 1), -alpha, depth - 1, true, new_pv, td);

                // re-search failed
                if (score > alpha && score < beta)
                {
                    score = -alpha_beta<~C>(board, ply + 1, -beta, -alpha, depth - 1, true, new_pv, td);
                }
            }
        }
#endif

#ifdef PVS
        // PVS

        if (best_score == -INF) // ou : legal_moves==1
        {
            score = -alpha_beta<~C>(board, ply + 1, -beta, -alpha, depth - 1, true, new_pv);
        } else {
            // On peut essayer la PVS (Principal Variation Search)
            score = -alpha_beta<~C>(board, ply + 1, -alpha - 1, -alpha, depth - 1, true, new_pv);

            // Est-ce que ça a marché ?
            if (!stopped && score > alpha && score < beta) {
                // Raté, il faut refaire une recherche complète
                score = -alpha_beta<~C>(board, ply + 1, -beta, -alpha, depth - 1, true, new_pv);
            }
        }
#endif

        // retract current move
        board.undo_move<C>();

        //  Time-out
        if (stopped)
            return 0;

        if (score >= beta)
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
                my_orderingInfo.updateKillers(ply, move);
                //TODO update history ?
            }
            Transtable.store(board.hash, move, score, HASH_BETA, depth, ply);
            return score;
        }

        // On a trouvé un bon coup
        if (score > best_score)
        {
            best_score = score; // le mieux que l'on ait trouvé, pas forcément intéressant

            if (score > alpha)
            {
                alpha = score;

                // alpha cutoff
                // https://www.chessprogramming.org/History_Heuristic
                if (Move::is_capturing(move) == false)
                {
                    //ZZ       searchHistory[C][Move::piece(move)][Move::dest(move)] += depth;
                    my_orderingInfo.incrementHistory(C, Move::piece(move), Move::dest(move), depth);
                }

                /* update the PV */
                update_pv(pv, new_pv, move);
                //                if (ply == 0)
                //                    show_uci_result(depth, best_score, 1, pv);

            } // score > alpha
        }     // meilleur coup
    }         // boucle sur les coups

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
            Transtable.store(board.hash, *pv, best_score, HASH_EXACT, depth, ply);
        else
            Transtable.store(board.hash, 0, best_score, HASH_ALPHA, depth, ply);
    }

    return best_score;
}

template void Search::think<WHITE>(int id);
template void Search::think<BLACK>(int id);

template int Search::alpha_beta<WHITE>(Board &board, int ply, int alpha, int beta, int depth, bool do_NULL, MOVE *pv, ThreadData* td);
template int Search::alpha_beta<BLACK>(Board &board, int ply, int alpha, int beta, int depth, bool do_NULL, MOVE *pv, ThreadData* td);
