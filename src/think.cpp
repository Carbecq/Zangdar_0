#include "MovePicker.h"
#include "Search.h"
#include "ThreadPool.h"
#include "Timer.h"
#include "TranspositionTable.h"
#include "Move.h"

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))


//======================================================
//! \brief  Lancement d'une recherche
//!
//! itérative deepening
//! alpha-beta
//!
//------------------------------------------------------
template<Color C>
void Search::think(const Board &m_board, const Timer &m_timer, int _index)
{
#if defined DEBUG_LOG
    char message[100];
    sprintf(message, "Search::think (thread=%d)", threadID);
    printlog(message);
#endif

    board = m_board;
    timer = m_timer;

    ThreadData* td = &threadPool.threadData[_index];

    // iterative deepening
    iterative_deepening<C>(td);

    if (_index == 0)
    {
        if (threadPool.get_logUci())
        {
            show_uci_best(td);
            timer.show_time();
        }

        threadPool.main_thread_stopped(); // arrêt des autres threads
        threadPool.wait(1);               // attente de ces threads
    }
}

//======================================================
//! \brief  iterative deepening
//!
//! alpha-beta
//!
//------------------------------------------------------
template<Color C>
void Search::iterative_deepening(ThreadData* td)
{
    PVariation pv;
    pv.length = 0;
    int  ply = 0;

    for (td->depth = 1; td->depth <= timer.getSearchDepth(); td->depth++)
    {
        // Search position, using aspiration windows for higher depths
        td->score = aspiration_window<C>(ply, pv, td);

        if (td->stopped)
            break;

        // L'itération s'est terminée sans problème
        // On peut mettre à jour les infos UCI
        if (td->index == 0)
        {
            bool uncertain = pv.line[0] != td->best_move;

            td->best_depth = td->depth;
            td->best_move  = pv.line[0];
            td->best_score = td->score;

            auto elapsed = timer.elapsedTime();

            if (threadPool.get_logUci())
                show_uci_result(td, elapsed, pv);

            // If an iteration finishes after optimal time usage, stop the search
            if (timer.finishOnThisDepth(elapsed, uncertain))
                break;

            td->seldepth = 0;
        }
    }
}

//======================================================
//! \brief  Aspiration Window
//!
//!
//------------------------------------------------------
template<Color C>
int Search::aspiration_window(int ply, PVariation& pv, ThreadData* td)
{
    int alpha  = -INFINITE;
    int beta   = INFINITE;
    int depth  = td->depth;
    int score  = td->score;

    const int initialWindow = 12;
    int delta = 16;

    // After a few depths use a previous result to form the window
    if (depth >= 6)
    {
        alpha = std::max(score - initialWindow, -INFINITE);
        beta  = std::min(score + initialWindow, INFINITE);
    }

    while (true)
    {
        score = alpha_beta<C>(ply, alpha, beta, std::max(1, depth), pv, td);

        if (td->stopped)
            break;

        // Search failed low, adjust window and reset depth
        if (score <= alpha)
        {
            alpha = std::max(score - delta, -INFINITE); // alpha/score-delta
            beta  = (alpha + beta) / 2;
            depth = td->depth;
        }

        // Search failed high, adjust window and reduce depth
        else if(score >= beta)  // Fail High
        {
            beta  = std::min(score + delta, INFINITE);   // beta/score+delta
            // idée de Berserk
            if (abs(score) < TBWIN_IN_X)
                depth--;
        }

        // Score within the bounds is accepted as correct
        else
            return score;

        delta += delta*2 / 3;
    }

    return score;
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
int Search::alpha_beta(int ply, int alpha, int beta, int depth, PVariation& pv, ThreadData* td)
{
    assert(board.valid<C>());
    assert(beta > alpha);

    SearchInfo* si = &td->info;

    // Update node count and selective depth
    td->nodes++;
    if (ply > td->seldepth)
        td->seldepth = ply;

    PVariation new_pv;
    new_pv.length = 0;
    pv.length     = 0;

    // On a atteint la limite en profondeur de recherche ?
    if (ply >= MAX_PLY - 1)
        return board.evaluate<true>();

    // On a atteint la limite de taille de la partie ?
    if (board.gamemove_counter >= MAX_HIST - 1)
        return board.evaluate<true>();

    //  Time-out
    if (td->stopped || check_limits(td))
    {
        td->stopped = true;
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
        return (quiescence<C>(ply, alpha, beta, td));
    }

    if (!isRoot)
    {
        //  Position nulle ?
        if (board.is_draw(ply))
            return CONTEMPT;

        // Mate distance pruning
        alpha = std::max(alpha, -MATE + ply);
        beta  = std::min(beta,   MATE - ply - 1);
        if (alpha >= beta)
            return alpha;
    }

    //  Recherche de la position actuelle dans la table de transposition
    Score tt_score;
    Score tt_eval;
    MOVE  tt_move;
    int   tt_flag;
    int   tt_depth;
    bool  tt_hit   = transpositionTable.probe(board.hash, ply, tt_move, tt_score, tt_eval, tt_flag, tt_depth);

    if (tt_hit)
    {
        // Trust TT if not a pvnode and the entry depth is sufficiently high
        if (   (tt_depth >= depth && !isPVNode)
            && (   (tt_flag & BOUND_EXACT)
                || (tt_flag & BOUND_LOWER && tt_score >= beta)
                || (tt_flag & BOUND_UPPER && tt_score <= alpha)))
        {
            return tt_score;
        }
    }

    // Probe the Syzygy     Tablebases
    int tbScore, tbEval, tbBound;
    if (threadPool.get_useSyzygy() && board.probe_wdl(tbScore, tbBound, ply) == true)
    {
        td->tbhits++;

        if (    tbBound & BOUND_EXACT
            || (tbBound & BOUND_LOWER ? tbScore >= beta : tbScore <= alpha))
        {
            transpositionTable.store(board.hash, Move::MOVE_NONE, tbScore, tbEval, tbBound, depth, MAX_PLY);
            return tbScore;
        }


        // Limit the score of this node based on the tb result
        if (isPVNode)
        {
            // Never score something worse than the known Syzygy value
            if (tbBound & BOUND_LOWER)
            {
                best_score = tbScore;
                alpha = std::max(alpha, tbScore);
            }
            else
            // Never score something better than the known Syzygy value
            {
                max_score = tbScore;
            }
        }
    }


    //  Evaluation Statique
    int static_eval;

    if (inCheck)
    {
        si->eval[ply] = static_eval = -MATE + ply;
    }
    else
    {
        si->eval[ply] = static_eval = board.evaluate<true>();
    }
    if (tt_hit)
        si->eval[ply] = static_eval = tt_eval;


    /*  Avons-nous amélioré la position ?
        Si on ne s'est pas amélioré dans cette ligne, on va pouvoir couper un peu plus */
    bool improving = false;
    if (ply > 2)
        improving = !inCheck && (static_eval > si->eval[ply - 2]);


    //  Controle si on va pouvoir utiliser des techniques de coupe pre-move
    int  score;

    if (!inCheck && !isRoot && !isPVNode)
    {
#if defined USE_RAZORING
        //---------------------------------------------------------------------
        //  RAZORING
        //---------------------------------------------------------------------
        if (   depth <= 3
            && (static_eval + 200 * depth) <= alpha)
        {
            score = quiescence<C>(ply, alpha, beta, td);
            if (score <= alpha)
            {
                td->nodes--;
                return score;
            }
        }
#endif

#if defined USE_REVERSE_FUTILITY_PRUNING
        //---------------------------------------------------------------------
        //  STATIC NULL MOVE PRUNING ou aussi REVERSE FUTILITY PRUNING
        //---------------------------------------------------------------------
        if (
            depth <= 6
            && abs(beta) < MATE_IN_X
            && board.getNonPawnMaterial<C>())
        {
            int eval_margin = 70 * depth;

            if (static_eval - eval_margin >= beta)
                return static_eval - eval_margin; // Fail Soft
        }
#endif

#if defined USE_NULL_MOVE_PRUNING
        //---------------------------------------------------------------------
        //  NULL MOVE PRUNING
        //---------------------------------------------------------------------
        if (
            depth >= 3
            && static_eval >= beta
            && board.non_pawn_count<C>() > 0
            && board.game_history[board.gamemove_counter-1].move != Move::MOVE_NONE)
        {
            int R = 3 + depth / 5 + std::min(3, (static_eval - beta)/256); //ZZZEVAL

            board.make_nullmove<C>();
            score = -alpha_beta<~C>(ply + 1, -beta, -beta + 1, depth - 1 - R, new_pv, td);
            board.undo_nullmove<C>();

            if (td->stopped)
                return 0;

            // Cutoff
            if (score >= beta)
            {
                // (Stockfish) : on ne retourne pas un score proche du mat
                //               car ce score ne serait pas prouvé
                return(score >= TBWIN_IN_X ? beta : score);
            }
        }
#endif

#if defined USE_PROBCUT
        //---------------------------------------------------------------------
        //  ProbCut
        //---------------------------------------------------------------------
        if (depth >= 5
            && abs(beta) < TBWIN_IN_X
            && !(   tt_hit
                 && tt_flag & BOUND_UPPER
                 && tt_score < beta))
        {
            int threshold = beta + 200;

            MovePicker movePicker(&board, si, Move::MOVE_NONE, Move::MOVE_NONE, Move::MOVE_NONE, true, 0);
            MOVE pbMove;

            while ( (pbMove = movePicker.next_move() ) != Move::MOVE_NONE )
            {
                board.make_move<C>(pbMove);

                // See if a quiescence search beats pbBeta
                int pbScore = -quiescence<~C>(ply+1, -threshold, -threshold+1, td);

                // If it did, do a proper search with reduced depth
                if (pbScore >= threshold)
                    pbScore = -alpha_beta<~C>(ply+1, -threshold, -threshold + 1, depth-4, new_pv, td);

                board.undo_move<C>();

                // Cut if the reduced depth search beats pbBeta
                if (pbScore >= threshold)
                {
                    // Store pbScore in TT ??
                    return pbScore;
                }
            }
        }
#endif
    } // end Pruning

#if defined USE_INTERNAL_ITERATIVE_DEEPENING
    //---------------------------------------------------------------------
    // Internal Iterative Deepening.
    //---------------------------------------------------------------------
    if (   tt_move == Move::MOVE_NONE
        && depth >= 4)
    {
        depth--;
    }
#endif

    //====================================================================================
    //  Génération des coups
    //------------------------------------------------------------------------------------

    MovePicker movePicker(&board, si, tt_move, si->killer1[ply], si->killer2[ply], false, 0);
    MOVE move;
    const int old_alpha = alpha;
    int moveCount = 0, quietCount = 0;

    // Boucle sur tous les coups
    while ( (move = movePicker.next_move() ) != Move::MOVE_NONE )
    {
        bool isQuiet = !Move::is_tactical(move);    // capture, promotion (avec capture ou non), prise en-passant

       int extension = 0;

#if defined USE_SINGULAR_EXTENSION
        //-------------------------------------------------
        //  SINGULAR EXTENSION
        //-------------------------------------------------


#endif

#if defined USE_LATE_MOVE_PRUNING
        //-------------------------------------------------
        //  Late Move Pruning / Move Count Based Pruning
        //-------------------------------------------------
        if (   !isPVNode
            && !inCheck
            && best_score > -TBWIN_IN_X
            && moveCount > (3 + 2 * depth * depth) / (2 - improving) )
            continue;
#endif

        // execute current move
        board.make_move<C>(move);

        // Update counter of moves actually played
        moveCount++;
        quietCount += isQuiet;

#if defined USE_LATE_MOVE_REDUCTION
        //------------------------------------------------------------------------------------
        //  LATE MOVE REDUCTION
        //------------------------------------------------------------------------------------
        int newDepth = depth - 1 + extension;
        bool doFullDepthSearch;

        if (   depth > 2
            && moveCount > (2 + isPVNode)
            && !inCheck
            && isQuiet)
        {
            // Base reduction
            int R = Reductions[isQuiet][std::min(31, depth)][std::min(31, moveCount)];
            // Reduce less in pv nodes
            R -= isPVNode;
            // Reduce less when improving
            R -= improving;

            // Depth after reductions, avoiding going straight to quiescence
            int lmrDepth = CLAMP(newDepth - R, 1, newDepth - 1);

            // Search this move with reduced depth:
            score = -alpha_beta<~C>(ply+1, -alpha-1, -alpha, lmrDepth, new_pv, td);

            doFullDepthSearch = score > alpha && lmrDepth < newDepth;
        } else
            doFullDepthSearch = !isPVNode || moveCount > 1;

        // Full depth zero-window search
        if (doFullDepthSearch)
            score = -alpha_beta<~C>(ply+1, -alpha-1, -alpha, newDepth, new_pv, td);

        // Full depth alpha-beta window search
        if (isPVNode && ((score > alpha && score < beta) || moveCount == 1))
            score = -alpha_beta<~C>(ply+1, -beta, -alpha, newDepth, new_pv, td);
#endif


        // retract current move
        board.undo_move<C>();

        //  Time-out
        if (td->stopped)
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
                    si->incrementHistory(C, move, depth);

                // If score beats beta we have a cutoff
                if (score >= beta)
                {
                    // non, ce coup est trop bon pour l'adversaire
                    // Update Killers
                    if (isQuiet)
                    {
                        si->updateKillers(ply, move);
                    }
                    transpositionTable.store(board.hash, move, score, static_eval, BOUND_LOWER, depth, ply);
                    return score;
                }


            } // score > alpha
        }     // meilleur coup
    }         // boucle sur les coups

    // est-on mat ou pat ?
    if (moveCount == 0)
    {
        // On est en échec, et on n'a aucun coup : on est MAT
        // On n'est pas en échec, et on n'a aucun coup : on est PAT
        return inCheck ? -MATE + ply : 0;
    }

    // don't let our score inflate too high (tb)
    best_score = std::min(best_score, max_score);

    if (!td->stopped)
    {
        //  si on est ici, c'est que l'on a trouvé au moins 1 coup
        //  et de plus : score < beta
        //  si score >  alpha    : c'est un bon coup : HASH_EXACT
        //  si score <= alpha    : c'est un coup qui n'améliore pas alpha : HASH_ALPHA
        int flag = (alpha != old_alpha) ? BOUND_EXACT : BOUND_UPPER;
        transpositionTable.store(board.hash, best_move, best_score, static_eval, flag, depth, ply);
    }

    return best_score;
}

template void Search::think<WHITE>(const Board &m_board, const Timer &m_timer, int _index);
template void Search::think<BLACK>(const Board &m_board, const Timer &m_timer, int _index);

template void Search::iterative_deepening<WHITE>(ThreadData* td);
template void Search::iterative_deepening<BLACK>(ThreadData* td);

template int Search::aspiration_window<WHITE>(int ply, PVariation& pv, ThreadData* td);
template int Search::aspiration_window<BLACK>(int ply, PVariation& pv, ThreadData* td);

template int Search::alpha_beta<WHITE>(int ply, int alpha, int beta, int depth, PVariation& pv, ThreadData* td);
template int Search::alpha_beta<BLACK>(int ply, int alpha, int beta, int depth, PVariation& pv, ThreadData* td);
