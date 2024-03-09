#ifndef ORDERINFO_H
#define ORDERINFO_H

class OrderInfo;

#include "defines.h"
#include "types.h"

//! \brief  Données utilisées au cours de recherche
class OrderInfo
{
public:
    OrderInfo();

    MOVE   killer1[MAX_PLY+2];                      // killer moves
    MOVE   killer2[MAX_PLY+2];
    MOVE   excluded[MAX_PLY+2];

    int    history[N_COLORS][N_PIECES][N_SQUARES];  // bonus history
    MOVE   counter[N_COLORS][N_PIECES][N_SQUARES];  // counter move

    void clear_all();
    void clear_killers();
    void update_killers(int ply, MOVE move);
    void update_history(Color color, MOVE move, int delta);
    int  get_history(const Color color, const MOVE move) const;
    void update_counter(Color color, int ply, MOVE prev_move, MOVE move);
    void get_refutation_moves(Color color, int ply, MOVE prev_move, MOVE& _killer_1, MOVE& _killer_2, MOVE& _counter_move);


}__attribute__((aligned(64)));

#endif // ORDERINFO_H
