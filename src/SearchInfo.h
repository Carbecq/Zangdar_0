#ifndef SEARCHINFO_H
#define SEARCHINFO_H

class SearchInfo;

#include "defines.h"
#include "types.h"

class SearchInfo
{
public:
    SearchInfo();

    MOVE   killer1[MAX_PLY+2]; //      = {Move::MOVE_NONE};        // killer moves
    MOVE   killer2[MAX_PLY+2]; //      = {Move::MOVE_NONE};
    int    history[N_COLORS][N_PIECES][N_SQUARES]; // = {{{0}}};   // bonus history
    Score  eval[MAX_PLY]    ; //        = {0};                      // evaluation statique


    void clearAllHistory();
    void updateKillers(int ply, MOVE move);
    void incrementHistory(Color color, MOVE move, int delta);
    int  get_history(const Color color, const MOVE move) const;


}__attribute__((aligned(64)));

#endif // SEARCHINFO_H
