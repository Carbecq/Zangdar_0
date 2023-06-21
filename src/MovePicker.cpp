#include "MovePicker.h"
#include "evaluate.h"
#include "types.h"


//=====================================================
//! \brief  Constructeur
//-----------------------------------------------------
MovePicker::MovePicker(int ply, const MOVE tt_move,
                       const OrderingInfo *m_orderingInfo, Board *m_board, MoveList *m_moveList)
{
    move_list    = m_moveList;
    board        = m_board;
    orderingInfo = m_orderingInfo;
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
    MOVE Killer1  = orderingInfo->getKiller1(ply);
    MOVE Killer2  = orderingInfo->getKiller2(ply);

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
            move_list->values[index] = PROMOTION_BONUS + mg_value[Move::piece(move)];
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
            PieceType piece = Move::piece(move);
            int dest = Move::dest(move);
            move_list->values[index] = QUIET_BONUS + orderingInfo->getHistory(board->side_to_move, piece, dest);
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
//! \brief  Recherche le meilleur coup à partir de currHead
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
//void MovePicker::verify_MvvLva()
//{
//    for(int Victim = PieceType::Pawn; Victim <= PieceType::King; ++Victim)
//    {
//        for(int Attacker = PieceType::Pawn; Attacker <= PieceType::King; ++Attacker)
//        {
//            printf("%10s prend %10s = %d\n", pchar[Attacker].c_str(), pchar[Victim].c_str(), MvvLvaScores[Victim][Attacker]);
//        }
//    }

//}




