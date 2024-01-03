#include "SearchInfo.h"
#include "Move.h"
#include <cstring>

SearchInfo::SearchInfo()
{
}

//=========================================================
//! \brief  Re-initialise tous les heuristiques
//---------------------------------------------------------
void SearchInfo::clearAllHistory()
{
    std::memset(history, 0, sizeof(history));
    std::memset(killer1, 0, sizeof(killer1));
    std::memset(killer2, 0, sizeof(killer2));
}


//=========================================================
//! \brief  Met à jour les heuristiques "Killer"
//---------------------------------------------------------
void SearchInfo::updateKillers(int ply, MOVE move)
{
    if (killer1[ply] != move)
    {
        killer2[ply] = killer1[ply];
        killer1[ply] = move;
    }
}

//=========================================================
//! \brief  Incrémente l'heuristique "History"
//---------------------------------------------------------
void SearchInfo::incrementHistory(Color color, MOVE move, int delta)
{
    int piece = Move::piece(move);
    int dest  = Move::dest(move);

    history[color][piece][dest] += delta;
}

//=========================================================
//! \brief  Récupère l'heuristique "History"
//---------------------------------------------------------
int SearchInfo::get_history(const Color color, const MOVE move) const
{
    return(history[color][Move::piece(move)][Move::dest(move)]);
}

