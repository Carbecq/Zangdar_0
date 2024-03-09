#include "OrderInfo.h"
#include "Move.h"
#include <cstring>

OrderInfo::OrderInfo()
{
    clear_all();
}

//=========================================================
//! \brief  Re-initialise tous les heuristiques
//---------------------------------------------------------
void OrderInfo::clear_all()
{
    std::memset(killer1,  0, sizeof(killer1));
    std::memset(killer2,  0, sizeof(killer2));
    std::memset(history,  0, sizeof(history));
    std::memset(counter,  0, sizeof(counter));
    std::memset(excluded, 0, sizeof(excluded));

}

//=========================================================
//! \brief  Re-initialise tous les killers
//---------------------------------------------------------
void OrderInfo::clear_killers()
{
    std::memset(killer1, 0, sizeof(killer1));
    std::memset(killer2, 0, sizeof(killer2));
}

//=========================================================
//! \brief  Met à jour les heuristiques "Killer"
//---------------------------------------------------------
void OrderInfo::update_killers(int ply, MOVE move)
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
void OrderInfo::update_history(Color color, MOVE move, int bonus)
{
    history[color][Move::piece(move)][Move::dest(move)] += bonus;
}

//=========================================================
//! \brief  Récupère l'heuristique "History"
//---------------------------------------------------------
int OrderInfo::get_history(const Color color, const MOVE move) const
{
    return(history[color][Move::piece(move)][Move::dest(move)]);
}

//=================================================================
//! \brief  Update countermove.
//! \param[in] color        couleur du camp qui joue
//! \param[in] ply          profondeur courante de la recherche
//! \param[in] prev_move    coup recherché au ply précédant
//! \param[in] move         coup de réfutation
//-----------------------------------------------------------------
void OrderInfo::update_counter(Color color, int ply, MOVE prev_move, MOVE move)
{
    if (prev_move != Move::MOVE_NONE && prev_move != Move::MOVE_NULL)
        counter[color][Move::piece(prev_move)][Move::dest(prev_move)] = move;
}

//==================================================================
//! \brief  Récupère les coups de réfutation
//! \param[in] color        couleur du camp qui joue
//! \param[in] ply          profondeur courante de la recherche
//! \param[in] prev_move    coup recherché au ply précédant
//------------------------------------------------------------------
void OrderInfo::get_refutation_moves(Color color, int ply, MOVE prev_move, MOVE& _killer_1, MOVE& _killer_2, MOVE& _counter_move)
{
    _counter_move = (prev_move==Move::MOVE_NONE || prev_move==Move::MOVE_NULL)
                        ? Move::MOVE_NONE
                        : counter[color][Move::piece(prev_move)][Move::dest(prev_move)];
    _killer_1 = killer1[ply];
    _killer_2 = killer2[ply];
}

