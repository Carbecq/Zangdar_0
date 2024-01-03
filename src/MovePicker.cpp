#include "MovePicker.h"
#include "types.h"
#include "Move.h"
#include "evaluate.h"

//=====================================================
//! \brief  Constructeur
//-----------------------------------------------------
MovePicker::MovePicker(Board* _board, const SearchInfo* _search_info,
                       MOVE _ttMove, MOVE _killer1, MOVE _killer2,
                       bool _skipQuiets, int _threshold) :
    board(_board),
    search_info(_search_info),
    skipQuiets(_skipQuiets),
    stage(STAGE_TABLE),
    gen_quiet(false),
    gen_legal(false),
    threshold(_threshold),
    tt_move(_ttMove),
    killer1(_killer1),
    killer2(_killer2)
{

}


//=====================================================
//! \brief  Sélection du prochain coup
//-----------------------------------------------------
MOVE MovePicker::next_move()
{
    switch (stage)
    {

    case STAGE_TABLE:

        // Play the table move if it is from this
        // position, also advance to the next stage

        stage = STAGE_GENERATE_NOISY;
        if (is_legal(tt_move))
            return tt_move;

        /* fallthrough */

    case STAGE_GENERATE_NOISY:

        // Generate all noisy moves and evaluate them. Set up the
        // split in the array to store quiet and noisy moves. Also,
        // this stage is only a helper. Advance to the next one.

        if (board->turn() == WHITE)
            board->legal_noisy<WHITE>(mln);
        else
            board->legal_noisy<BLACK>(mln);

        score_noisy();
        stage = STAGE_GOOD_NOISY ;

        /* fallthrough */

    case STAGE_GOOD_NOISY:

        // Check to see if there are still more noisy moves
        if (mln.count != 0)
        {
            int  best     = get_best(mln);
            MOVE bestMove = mln.moves[best];

            // Don't play the table move twice
            if (bestMove == tt_move)
            {
                pop_move(mln, best);
                return next_move();
            }

            if (!board->fast_see(bestMove, threshold))
            {
                shift_bad(best);
                return next_move();
            }

            return pop_move(mln, best);
        }

        if (skipQuiets)
        {
            stage = STAGE_BAD_NOISY;
            return next_move();
        }

        stage = STAGE_KILLER_1;

        /* fallthrough */

    case STAGE_KILLER_1:

        // Play the killer move if it is from this position.
        // position, and also advance to the next stage
        stage = STAGE_KILLER_2;

        if (!skipQuiets && killer1 != tt_move)
        {
            if (gen_quiet == false)
            {
                if (board->turn() == WHITE)
                    board->legal_quiet<WHITE>(mlq);
                else
                    board->legal_quiet<BLACK>(mlq);
                gen_quiet = true;
                score_quiet();
            }
            if (is_legal(killer1))
                return killer1;
        }

        /* fallthrough */

    case STAGE_KILLER_2:

        // Play the killer move if it is from this position.
        // position, and also advance to the next stage
        stage = STAGE_GENERATE_QUIET;

        if (!skipQuiets && killer2 != tt_move)
        {
            if (gen_quiet == false)
            {
                if (board->turn() == WHITE)
                    board->legal_quiet<WHITE>(mlq);
                else
                    board->legal_quiet<BLACK>(mlq);
                gen_quiet = true;
                score_quiet();
            }
            if (is_legal(killer2))
                return killer2;
        }

        /* fallthrough */

    case STAGE_GENERATE_QUIET:

        // Generate all quiet moves and evaluate them
        // and also advance to the final fruitful stage
        if (!skipQuiets)
        {
            if (gen_quiet == false)
            {
                if (board->turn() == WHITE)
                    board->legal_quiet<WHITE>(mlq);
                else
                    board->legal_quiet<BLACK>(mlq);
                gen_quiet = true;
                score_quiet();
            }
        }
        stage = STAGE_QUIET;

        /* fallthrough */

    case STAGE_QUIET:

        // Check to see if there are still more quiet moves
        if (mlq.count > 0 && !skipQuiets)
        {
            int  best     = get_best(mlq);
            MOVE bestMove = pop_move(mlq, best);

            if (   bestMove == tt_move
                || bestMove == killer1
                || bestMove == killer2  /*|| m == moves->counter */)
                return next_move();
            else
                return bestMove;
        }

        // If no quiet moves left, advance stages
        stage = STAGE_BAD_NOISY;

        /* fallthrough */

    case STAGE_BAD_NOISY:

        if (mlb.count > 0)
        {
            int  best     = get_best(mlb);
            MOVE bestMove = pop_move(mlb, best);

            // Don't play the table move twice
            if (bestMove == tt_move)
                return next_move();
            return bestMove;
        }

        stage = STAGE_DONE;
        //    return Move::MOVE_NONE;


    case STAGE_DONE:
        return Move::MOVE_NONE;

    default:
        assert(0);
        return Move::MOVE_NONE;
    }
}

//==================================================================
//! \brief  Evaluation des coups de capture et de promotion
//------------------------------------------------------------------
void MovePicker::score_noisy()
{
    MOVE move;
    int  value;

    for (size_t i = 0; i < mln.count; i++)
    {
        move     = mln.moves[i];

        // Use the standard MVV-LVA
        // PieceType dest_type = board->piece_on(Move::dest(move));  // pièce prise ou promotion

        // std::cout << "i= " << i << "  " << Move::name(move) << std::endl;
        // std::cout << "captured = " << piece_name[Move::captured(move)] << " value = " << MvvLvaScores[Move::captured(move)][Move::piece(move)] << std::endl;
        // std::cout << "dest     = " << piece_name[dest_type] << " value = "            << MvvLvaScores[dest_type][Move::piece(move)] << std::endl;
        // std::cout << "promo    = " << piece_name[Move::promotion(move)] << " value = " << MvvLvaScores[Move::promotion(move)][Move::piece(move)] << std::endl;


        //    value = mg_value[dest_type] - Move::piece(move);
        value = MvvLvaScores[Move::captured(move)][Move::piece(move)];

        // A bonus is in order for queen promotions
        if (Move::is_promoting(move))
            value += eg_value[Move::promotion(move)];

        // Enpass is a special case of MVV-LVA
        else if (Move::is_enpassant(move))
            value = MvvLvaScores[Pawn][Pawn];
                // eg_value[PieceType::Pawn] - PieceType::Pawn;

        mln.values[i] = value;
    }
}

//==================================================================
//! \brief  Evaluation des coups tranquilles
//------------------------------------------------------------------
void MovePicker::score_quiet()
{
    // Use the History score from the Butterfly Bitboards for sorting
    for (size_t i = 0; i < mlq.count; i++)
        mlq.values[i] = search_info->get_history(board->turn(), mlq.moves[i]);
}

//====================================================
//! \brief  Retourne l'indice du meilleur élément
//-----------------------------------------------------
int MovePicker::get_best(const MoveList& ml)
{
    int best_index = 0;

    // Find highest scoring move
    for (int i = 1; i < ml.count; i++)
    {
        if (ml.values[i] > ml.values[best_index])
            best_index = i;
    }

    return best_index;
}
//========================================================
//! \brief  Retourne le coup indiqué
//! puis déplace le dernier élément à la position
//! du coup indiqué
//--------------------------------------------------------
MOVE MovePicker::pop_move(MoveList& ml, int idx)
{
    MOVE temp = ml.moves[idx];

    ml.count--;
    ml.moves[idx]  = ml.moves[ml.count];
    ml.values[idx] = ml.values[ml.count];

    return temp;
}

//======================================================
//! \brief  Déplace le coup indiqué dans la liste mlb
//! Puis enlève le coup de la liste mln
//------------------------------------------------------
void MovePicker::shift_bad(int idx)
{
    // Put the bad capture in the "bad" list
    mlb.moves[mlb.count]  = mln.moves[idx];
    mlb.values[mlb.count] = mln.values[idx];
    mlb.count++;

    // put the last good capture here instead
    mln.count--;
    mln.moves[idx]  = mln.moves[mln.count];
    mln.values[idx] = mln.values[mln.count];
}


//==================================================================
//! \brief  Vérification de la légalité d'un coup
//------------------------------------------------------------------
bool MovePicker::is_legal(MOVE move)
{
    if (/*move == NULL_MOVE ||*/ move == Move::MOVE_NONE)
        return false;

    //ZZZZ  Peut-on faire plus rapide ???

    if (gen_legal == false)
    {
        if (board->turn() == WHITE)
            board->legal_moves<WHITE>(mll);
        else
            board->legal_moves<BLACK>(mll);
        gen_legal = true;
    }

    for (int n=0; n<mll.count; n++)
        if (mll.moves[n] == move)
            return true;
    return false;
}

//==============================================
//  Ancien MovePicker
//==============================================

//=====================================================
//! \brief  Constructeur
//-----------------------------------------------------
MovePicker::MovePicker(int ply, const MOVE tt_move,
                       const SearchInfo *m_SearchInfo, Board *m_board, MoveList *m_moveList)
{
    move_list    = m_moveList;
    board        = m_board;
    search_info  = m_SearchInfo;
    currHead     = 0;

    scoreMoves(ply, tt_move);
}

//=========================================================
//! \brief  Donne un bonus aux coups, de façon à les trier
//!
//! A typical move ordering consists as follows:
//!
//! 1) PV-move of the principal variation from the previous iteration
//!     of an iterative deepening framework for the leftmost path, often implicitly done by (2).
//! 2) Hash move from hash tables
//! 3) Winning captures/promotions
//! 4) Equal captures/promotions
//! 5) Killer moves (non capture), often with mate killers first
//! 6) Non-captures sorted by history heuristic and that like
//! 7) Losing captures (* but see below)
//!
//! https://www.chessprogramming.org/Move_Ordering
//---------------------------------------------------------
void MovePicker::scoreMoves(int ply, const MOVE tt_move)
{
    MOVE Killer1  = search_info->killer1[ply];
    MOVE Killer2  = search_info->killer2[ply];

    for (size_t index=0; index<move_list->size(); index++)
    {
        MOVE move = move_list->moves[index];

        if (tt_move != 0 && move == tt_move)
        {
            move_list->values[index] = ( 1000000 );
        }
        else if (Move::is_capturing(move))
        {
            //            if (captured > piece)
            //            {
            //                move_list->values[index] = GOOD_CAPTURE + MvvLvaScores[captured][piece];
            //            }
            //            else
            //            {
            //                //int v = board->see(move);
            //                bool b = board->fast_see(move, 0);
            //                if (b)
            //                    move_list->values[index] = GOOD_CAPTURE + MvvLvaScores[captured][piece];
            //                else
            //                    move_list->values[index] = BAD_CAPTURE + MvvLvaScores[captured][piece];
            //            }
            move_list->values[index] = GOOD_CAPTURE + MvvLvaScores[Move::captured(move)][Move::piece(move)];
        }
        //       else if (Move::is_enpassant(move))
        //        {
        //            move_list->values[index] = GOOD_CAPTURE + MvvLvaScores[Pawn][Pawn];
        //        }
        else if (Move::is_promoting(move))
        {
            move_list->values[index] = PROMOTION_BONUS + mg_value[Move::promotion(move)];
        }
        else if (move == Killer1)
        {
            move_list->values[index] = KILLER1_BONUS;
        }
        else if (move == Killer2)
        {
            move_list->values[index] = KILLER2_BONUS;
        }
        else
        { // Quiet
            // PieceType piece = Move::piece(move);
            // int dest = Move::dest(move);
            move_list->values[index] = QUIET_BONUS + search_info->get_history(board->side_to_move, move);
        }
    }
}

//============================================================
//! \brief  Retourne true s'il reste des coups à retirer
//------------------------------------------------------------
bool MovePicker::hasNext() const
{
    return currHead < move_list->size();
}

//=============================================================
//! \brief  Recherche le meilleur coup à partir de next
//-------------------------------------------------------------
MOVE MovePicker::getNext()
{
    size_t bestIndex = currHead;
    int    bestScore = -INFINITE;

    for (size_t index = currHead; index < move_list->count; index++)
    {
        if (move_list->values[index] > bestScore)
        {
            bestScore = move_list->values[index];
            bestIndex = index;
        }
    }

    if (currHead != bestIndex)
        move_list->swap(currHead, bestIndex);

    return(move_list->moves[currHead++]);
}


std::string pchar[7] = {"NoPiece", "Pion", "Cavalier", "Fou", "Tour", "Dame", "Roi"};
void MovePicker::verify_MvvLva()
{
   for(int Victim = PieceType::Pawn; Victim <= PieceType::King; ++Victim)
   {
       for(int Attacker = PieceType::Pawn; Attacker <= PieceType::King; ++Attacker)
       {
           printf("%10s prend %10s = %d\n", pchar[Attacker].c_str(), pchar[Victim].c_str(), MvvLvaScores[Victim][Attacker]);
       }
   }

}
