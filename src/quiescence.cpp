#include "Search.h"
#include "MovePicker.h"
#include "OrderingInfo.h"

// http://talkchess.com/forum3/viewtopic.php?f=7&t=79828&p=925394&hilit=quiescence+check&sid=e2fc0eb4370c4559e0e1377dc25e6730#p925394
//extern ThreadData   cData[MAX_THREADS];

//=============================================================
//! \brief  Recherche jusqu'à obtenir une position calme,
//!         donc sans prise ou promotion.
//-------------------------------------------------------------
template <Color C>
int Search::quiescence(Board &board, int ply, int alpha, int beta, MOVE* pv)
{
    assert(beta > alpha);
    MOVE new_pv[MAX_PLY] = {0};

    nodes++;

    //  Time-out
    if (stopped || check_limits()) {
        stopped = true;
        return 0;
    }

    *pv = 0;

    // partie nulle ?
    if(board.is_draw<C>())
        return 0;

    // profondeur de recherche max atteinte
    // prevent overflows
    if (ply >= MAX_PLY - 1)
        return board.is_in_check<C>() ? 0 : board.evaluate();

    // partie trop longue
    if (board.game_clock >= MAX_HIST - 1)
        return board.evaluate();

    int  best_score = -INF;
    int  score;

    //TODO passer in_check en argument
    bool in_check = board.is_in_check<C>();
    MoveList move_list;

    // stand pat
    if (!in_check)
    {
        // you do not allow the side to move to stand pat if the side to move is in check.
        best_score = board.evaluate();

        // le score est trop mauvais pour moi, on n'a pas besoin
        // de chercher plus loin
        if (best_score >= beta)
            return best_score;

        // l'évaluation est meilleure que alpha. Donc on peut améliorer
        // notre position. On continue à chercher.
        if (best_score > alpha)
            alpha = best_score;

        board.legal_captures<C>(move_list);
    }
    else
    {
        board.legal_evasions<C>(move_list);
    }

    MovePicker movePicker(ply, 0, &my_orderingInfo, &board, &move_list);
    MOVE move;

    // Boucle sur tous les coups
    while (movePicker.hasNext())
    {
        move = movePicker.getNext();

        board.make_move<C>(move);
        score = -quiescence<~C>(board, ply+1, -beta, -alpha, new_pv);
        board.undo_move<C>();

        if (stopped)
            return 0;

        // try for an early cutoff:
        if(score >= beta)
        {
            /* we have a cutoff, so update our killers: */
            //TODO : utiliser killer ici ?
            if (Move::is_capturing(move) == false)
                my_orderingInfo.updateKillers(ply, move);
            return score;
        }

        if (score > best_score)
        {
            best_score = score;
            if (score > alpha)
            {
                alpha = score;
                update_pv(pv, new_pv, move);
            }
        }
    }

    // est-on mat ou pat ?
    if (best_score == -INF)
        return in_check ? -MATE + ply : 0;

    return best_score;  // return the best score found so far
}

template int Search::quiescence<WHITE>(Board &board, int ply, int alpha, int beta, MOVE* pv);
template int Search::quiescence<BLACK>(Board &board, int ply, int alpha, int beta, MOVE* pv);
