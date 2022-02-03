#include "Position.h"
#include <sstream>
#include <string>
#include "defines.h"
#include "Search.h"

Position::Position()
{
#ifdef DEBUG_CLASS
    std::cout << "Position::Position()" << std::endl;
#endif
    side_to_move = WHITE;
    ep_square    = OFFBOARD;
    ep_took      = OFFBOARD;
    castle       = CASTLE_NONE;
    fifty        = 0;
    hash         = 0;
    ply          = 0;
    hply         = 0;

}
void Position::init(Color c)
{
#ifdef DEBUG_CLASS
    std::cout << "Position::init()" << std::endl;
#endif
    side_to_move = c;
    ep_square    = OFFBOARD;
    ep_took      = OFFBOARD;
    castle       = CASTLE_NONE;
    fifty        = 0;
    hash         = 0;
    ply          = 0;
    hply         = 0;

}



void Position::reset()
{
#ifdef DEBUG_CLASS
    std::cout << "Position::reset()" << std::endl;
#endif
    ep_square = OFFBOARD;
    fifty     = 0;
    castle    = CASTLE_NONE;

    ply  = 0;
    hply = 0;

    hash = 0ULL;

}
//====================================================================
//! \brief  Détermine s'il y a eu répétition de la même position
//! Pour cela, on compare le hash code de la position.
//!
//! \return Renvoie le nombre de fois où la osition courante
//! a été répétée.
//--------------------------------------------------------------------
int Position::is_repetition(const std::array<History, MAX_HIST>& history) const
{
    int r = 0;

    for (int i = hply - fifty; i < hply; ++i)
    {
        if (history[i].hash == hash)
            r++;
    }

    return r;
}

//=============================================================================
//! \brief  Détermine si la position est nulle
//-----------------------------------------------------------------------------
bool Position::is_draw(const std::array<History, MAX_HIST>& history) const
{
    return((is_repetition(history) || fifty >= 100) && ply);
}
