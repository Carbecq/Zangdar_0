#include "Search.h"
#include "MovePicker.h"
#include "Move.h"

//=============================================================
//! \brief  Recherche jusqu'à obtenir une position calme,
//!         donc sans prise ou promotion.
//-------------------------------------------------------------
template <Color C>
int Search::quiescence(int ply, int alpha, int beta, ThreadData* td)
{
    assert(beta > alpha);

    SearchInfo* si = &td->info;

    //  Time-out
    if (td->stopped || check_limits(td))
    {
        td->stopped = true;
        return 0;
    }


    // Update node count and selective depth
    td->nodes++;
    if (ply > td->seldepth)
        td->seldepth = ply;


    // partie nulle ?
    if(board.is_draw(ply))
        return CONTEMPT;


    // profondeur de recherche max atteinte
    // prevent overflows
    bool in_check = board.is_in_check<C>();
    if (ply >= MAX_PLY - 1)
        return in_check ? 0 : board.evaluate<true>();


    // partie trop longue
    if (board.gamemove_counter >= MAX_HIST - 1)
        return board.evaluate<true>();


    int  best_score;
    int  score;

    // stand pat

    if (!in_check)
    {
        // you do not allow the side to move to stand pat if the side to move is in check.
        best_score = board.evaluate<true>();

        // le score est trop mauvais pour moi, on n'a pas besoin
        // de chercher plus loin
        if (best_score >= beta)
            return best_score;

        // l'évaluation est meilleure que alpha. Donc on peut améliorer
        // notre position. On continue à chercher.
        if (best_score > alpha)
            alpha = best_score;
    }
    else
    {
        best_score = -MATE + ply; // idée de Koivisto
    }
    
    MOVE move;
    MovePicker movePicker(&board, si, Move::MOVE_NONE, Move::MOVE_NONE, Move::MOVE_NONE, true, 0);

    // Boucle sur tous les coups
    while ((move = movePicker.next_move()) != Move::MOVE_NONE)
    {
        // Prune des prises inintéressantes
        if (!in_check && movePicker.get_stage() > STAGE_GOOD_NOISY)
            break;

        board.make_move<C>(move);
        score = -quiescence<~C>(ply+1, -beta, -alpha, td);
        board.undo_move<C>();

        if (td->stopped)
            return 0;

        // try for an early cutoff:
        if(score >= beta)
        {
            /* we have a cutoff, so update our killers: */
            if (Move::is_capturing(move) == false)
                si->updateKillers(ply, move);
            return score;
        }

        // Found a new best move in this position
        if (score > best_score)
        {
            best_score = score;

            // If score beats alpha we update alpha
            if (score > alpha)
                alpha = score;
        }
    }

    return best_score;
}

template int Search::quiescence<WHITE>(int ply, int alpha, int beta, ThreadData* td);
template int Search::quiescence<BLACK>(int ply, int alpha, int beta, ThreadData* td);
