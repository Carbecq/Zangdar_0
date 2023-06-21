#ifndef MOVEPICKER_H
#define MOVEPICKER_H

// idée de Drofa

class MovePicker;

#include "MoveList.h"
#include "Board.h"
#include "OrderingInfo.h"

constexpr int MvvLvaScores[7][7] = {
    {0, 16, 15, 14, 13, 12, 11}, // victim No_Type
    {0, 16, 15, 14, 13, 12, 11}, // victim Pawn
    {0, 26, 25, 24, 23, 22, 21}, // victim Knight
    {0, 36, 35, 34, 33, 32, 31}, // victim Bishop
    {0, 46, 45, 44, 43, 42, 41}, // vitcim Rook
    {0, 56, 55, 54, 53, 52, 51}, // victim Queen
    {0,  0,  0,  0,  0,  0,  0}  // victim King
};

/**
 * @brief Bonuses applied to specific move types.
 * @{
 */
constexpr int GOOD_CAPTURE       = 400000;
constexpr int PROMOTION_BONUS    = 300000;
constexpr int KILLER1_BONUS      = 200000;
constexpr int KILLER2_BONUS      = 150000;
constexpr int COUNTERMOVE_BONUS  =  50000;
constexpr int QUIET_BONUS        =      0;
constexpr int BAD_CAPTURE        = -20000;

/**
 * @brief Class for an object that picks moves from a move list in an optimal order.
 *
 * Implementations must define hasNext(), which returns true if there are remaining moves
 * to be processed and getNext(), which gets the next move from the provided MoveList.
 *
 * Note that instances of a MovePicker modify their provided MoveList instances in place.
 */
class MovePicker
{
public:
    MovePicker(int ply, const MOVE tt_move,
               const OrderingInfo *orderingInfo,
               Board *board,
               MoveList *moveList);

    bool hasNext() const;
    MOVE getNext();


private:
    MoveList*           move_list;
    const OrderingInfo* orderingInfo;
    Board*              board;
    size_t              currHead;

    void scoreMoves(int ply, const MOVE tt_move);


};

#endif // MOVEPICKER_H
