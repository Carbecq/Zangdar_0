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
    limits.movestogo = movestogo;
    limits.depth = depth;
    limits.nodes = nodes;
    limits.movetime = movetime;
    limits.infinite = infinite;

    timeForThisMove = 0;
    searchDepth = 0;
}

void Timer::reset()
{
    limits.time[WHITE] = 0;
    limits.time[BLACK] = 0;
    limits.incr[WHITE] = 0;
    limits.incr[BLACK] = 0;
    limits.movestogo = 0;
    limits.depth = 0;
    limits.nodes = 0;
    limits.movetime = 0;
    limits.infinite = false;

    timeForThisMove = 0;
    searchDepth = 0;
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

    searchDepth = MAX_PLY;
    timeForThisMove = MAX_TIME;

    if (limits.infinite) // recherche infinie (temps et profondeur)
    {
        searchDepth = MAX_PLY;
        timeForThisMove = MAX_TIME;
    }
    else if (limits.depth != 0) // profondeur de recherche imposée = depth
    {
        searchDepth = limits.depth;
        timeForThisMove = MAX_TIME;
    }
    else if (limits.movetime != 0) // temps de recherche imposé = move_time
    {
        searchDepth = MAX_PLY;
        timeForThisMove = limits.movetime;
    }
    else if (limits.time[color] != 0) {
        int time_left = limits.time[color];
        int increment = limits.incr[color];
        int movestogo = limits.movestogo;

        // partie : 40 coups en 15 minutes              : moves_to_go = 40 ; wtime=btime = 15*60000 ; winc=binc = 0
        // partie en 5 minutes, incrément de 6 secondes : moves_to_go = 0  ; wtime=btime =  5*60000 ; winc=binc = 6 >> sudden death

        // CCRL blitz :  2min base time + 1sec increment

        assert(time_left >= 0);
        assert(increment >= 0);
        assert(movestogo >= 0);

        // code inspiré par Fruit

        if (movestogo == 0)
            movestogo = 45;

        double time_max = std::max(0.0, time_left - BUFFER);
        double alloc    = (time_max + increment * static_cast<double>(movestogo - 1))
                          / static_cast<double>(movestogo);

        timeForThisMove = std::min(alloc, time_max);
    }


#ifdef DEBUG_TIME
    debug();
#endif
}

//==============================================================================
//! \brief Check if the time we alloted for picking this move has expired.
//------------------------------------------------------------------------------
bool Timer::checkLimits() const
{
    auto fin = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(fin - startTime).count();

    if (elapsed > timeForThisMove)
        return true;

    return false;
}

//===========================================================
//! \brief  Détermine si on a assez de temps pour effectuer
//!         une nouvelle itération
//! \param  elapsedTime     temps en millisecondes
//!
//  Idée de Drofa
//-----------------------------------------------------------
bool Timer::finishOnThisDepth(U64 &elapsed)
{
    auto fin = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(fin - startTime).count();

    //    std::cout << "finish : e=" << elapsed << " ; t=" << timeForThisMove << std::endl;
    // If we used 90% of the time so far, we break here
    if (elapsed > timeForThisMove * 0.9)
        return true;

    return false;
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
    std::cout << "timeForThisMove: " << timeForThisMove << " searchDepth: " << searchDepth
              << std::endl;
}
