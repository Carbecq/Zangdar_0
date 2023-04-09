#include "OrderingInfo.h"
#include <cstring>

//=========================================================
//! \brief  Constructeur
//---------------------------------------------------------
OrderingInfo::OrderingInfo()
{
    clearAllHistory();
}

//=========================================================
//! \brief  Re-initialise tous les heuristiques
//---------------------------------------------------------
void OrderingInfo::clearAllHistory()
{
    std::memset(history, 0, sizeof(history));
    std::memset(killer1, 0, sizeof(killer1));
    std::memset(killer2, 0, sizeof(killer2));
}

//=========================================================
//! \brief  Re-initialise tous les Killers
//---------------------------------------------------------
void OrderingInfo::clearKillers()
{
  std::memset(killer1, 0, sizeof(killer1));
  std::memset(killer2, 0, sizeof(killer2));
}

//=========================================================
//! \brief
//---------------------------------------------------------
MOVE OrderingInfo::getKiller1(int ply) const
{
    return killer1[ply];
}

//=========================================================
//! \brief
//---------------------------------------------------------
MOVE OrderingInfo::getKiller2(int ply) const
{
    return killer2[ply];
}

//=========================================================
//! \brief  Retourne la valeur de l'heuristique "History"
//---------------------------------------------------------
int OrderingInfo::getHistory(int color, PieceType piece, int dest) const
{
    return history[color][piece][dest];
}

//=========================================================
//! \brief  Incrémente l'heuristique "History"
//---------------------------------------------------------
void OrderingInfo::incrementHistory(int color, PieceType piece, int to, int depth)
{
    history[color][piece][to] += depth;
}

//=========================================================
//! \brief  Met à jour les heuristiques "Killer"
//---------------------------------------------------------
void OrderingInfo::updateKillers(int ply, MOVE move)
{
    if (killer1[ply] != move)
    {
        killer2[ply] = killer1[ply];
        killer1[ply] = move;
    }
}
