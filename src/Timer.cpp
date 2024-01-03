#include "Timer.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

Timer::Timer()
{
    reset();
}

Timer::Timer(bool infinite,
             int wtime,
             int btime,
             int winc,
             int binc,
             int movestogo,
             int depth,
             int nodes,
             int movetime)
{
    limits.time[WHITE] = wtime;
    limits.time[BLACK] = btime;
    limits.incr[WHITE] = winc;
    limits.incr[BLACK] = binc;
    limits.movestogo   = movestogo;
    limits.depth       = depth;
    limits.nodes       = nodes;
    limits.movetime    = movetime;
    limits.infinite    = infinite;

    timeForThisDepth    = 0;
    timeForThisMove     = 0;
    searchDepth         = 0;
}

void Timer::reset()
{
    limits.time[WHITE] = 0;
    limits.time[BLACK] = 0;
    limits.incr[WHITE] = 0;
    limits.incr[BLACK] = 0;
    limits.movestogo   = 0;
    limits.depth       = 0;
    limits.nodes       = 0;
    limits.movetime    = 0;
    limits.infinite    = false;

    timeForThisDepth    = 0;
    timeForThisMove     = 0;
    searchDepth         = 0;
}

//===========================================================
//! \brief Start the timer
//-----------------------------------------------------------
void Timer::start()
{
    startTime = std::chrono::high_resolution_clock::now();
}

//===========================================================
//! \brief Initialisation des limites en temps pour la recherche
//-----------------------------------------------------------
void Timer::setup(Color color)
{
    //        std::cout << "time_left   " << limits.time[WHITE] << "  " << limits.time[BLACK] << std::endl;
    //        std::cout << "increment   " << limits.incr[WHITE] << "  " << limits.incr[BLACK] << std::endl;
    //        std::cout << "moves_to_go " << limits.movestogo << std::endl;
    //        std::cout << "depth       " << limits.depth << std::endl;
    //        std::cout << "nodes       " << limits.nodes << std::endl;
    //        std::cout << "move_time   " << limits.movetime << std::endl;
    //        std::cout << "infinite    " << limits.infinite << std::endl;


    searchDepth         = MAX_PLY;
    timeForThisMove     = MAX_TIME;
    timeForThisDepth    = MAX_TIME;

    if (limits.infinite) // recherche infinie (temps et profondeur)
    {
        searchDepth         = MAX_PLY;
        timeForThisMove     = MAX_TIME;
        timeForThisDepth    = MAX_TIME;
    }
    else if (limits.depth != 0) // profondeur de recherche imposée = depth
    {
        searchDepth         = limits.depth;
        timeForThisMove     = MAX_TIME;
        timeForThisDepth    = MAX_TIME;
    }
    else if (limits.movetime != 0) // temps de recherche imposé = move_time
    {
        searchDepth         = MAX_PLY;
        timeForThisMove     = limits.movetime - BUFFER;
        timeForThisDepth    = limits.movetime - BUFFER;
    }
    else if (limits.time[color] != 0)
    {
        int time      = limits.time[color];
        int increment = limits.incr[color];
        int movestogo = limits.movestogo;

        // partie : 40 coups en 15 minutes              : moves_to_go = 40 ; wtime=btime = 15*60000 ; winc=binc = 0
        // partie en 5 minutes, incrément de 6 secondes : moves_to_go = 0  ; wtime=btime =  5*60000 ; winc=binc = 6 >> sudden death

        // CCRL blitz :  2min base time + 1sec increment

        // idée de Weiss

        // Plan as if there are at most 50 moves left to play with current time
        int mtg = movestogo ? std::min(movestogo, 50) : 50;

        int timeLeft = std::max(0, time
                                       + increment * mtg
                                       - BUFFER * mtg);

        if (!movestogo)
        {
            // Temps pour la partie entière
            double scale = 0.02;
            timeForThisDepth = std::min(timeLeft * scale, 0.2 * time);

        } else {
            // X coups en Y temps
            double scale = 0.7 / mtg;
            timeForThisDepth = std::min(timeLeft * scale, 0.8 * time);
        }

        timeForThisMove = std::min(5.0 * timeForThisDepth, 0.8 * time);
    }


#if defined DEBUG_TIME
    debug();
#endif
}

//==============================================================================
//! \brief Check if the time we alloted for picking this move has expired.
//------------------------------------------------------------------------------
bool Timer::finishOnThisMove() const
{
    auto fin = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(fin - startTime).count();

    return (elapsed >= timeForThisMove);
}

//==============================================================================
//! \brief  Retourne le temps écoulé depuis le début de la recherche.
//------------------------------------------------------------------------------
int Timer::elapsedTime()
{
    return (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count());
}

//===========================================================
//! \brief  Détermine si on a assez de temps pour effectuer
//!         une nouvelle itération
//! \param  elapsedTime     temps en millisecondes
//!
//-----------------------------------------------------------
bool Timer::finishOnThisDepth(U64 elapsed, bool uncertain)
{
    return (elapsed > timeForThisDepth * (1 + uncertain));
}

//==================================================================
//! \brief Affiche le temps passé en millisecondes
//------------------------------------------------------------------
void Timer::show_time()
{
    // Elapsed time in milliseconds
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - startTime).count();
    std::cout << "Time           " << ms / 1000.0 << " s" << std::endl;
}

//==================================================================
//! \brief Affiche des informations de debug
//------------------------------------------------------------------
void Timer::debug()
{
    std::cout << "timeForThisDepth: " << timeForThisDepth
              << " timeForThisMove: " << timeForThisMove
              << " searchDepth: " << searchDepth
              << std::endl;
}
