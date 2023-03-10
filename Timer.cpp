#include "Timer.h"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>
#include <cmath>
#include <cstring>
#include <algorithm>

Timer::Timer()
{
    reset();
}

Timer::Timer(bool infinite, int wtime, int btime, int winc, int binc, int movestogo,
             int depth, int nodes, int movetime)
{
    limits.time[WHITE]  = wtime;
    limits.time[BLACK]  = btime;
    limits.incr[WHITE]  = winc;
    limits.incr[BLACK]  = binc;
    limits.movestogo    = movestogo;
    limits.depth        = depth;
    limits.nodes        = nodes;
    limits.movetime     = movetime;
    limits.infinite     = infinite;

    timeForThisMove     = 0;
    searchDepth         = 0;
}

void Timer::reset()
{
    limits.time[WHITE]  = 0;
    limits.time[BLACK]  = 0;
    limits.incr[WHITE]  = 0;
    limits.incr[BLACK]  = 0;
    limits.movestogo    = 0;
    limits.depth        = 0;
    limits.nodes        = 0;
    limits.movetime     = 0;
    limits.infinite     = false;

    timeForThisMove     = 0;
    searchDepth         = 0;
}

// go depth 6 wtime 180000 btime 100000 binc 1000 winc 1000 movetime 1000 movestogo 40

void Timer::go(std::istringstream &is, Color side)
{
    int auxi;

    std::string token;
    token.clear();

    // We received a start command. Extract all parameters from the
    // command and start the search.

    while (is >> token)
    {
        if (token == "infinite")
        {
            // search until the "stop" command. Do not exit the search without being told so in this mode!
            limits.infinite = true;
        }
        else if (token == "wtime")
        {
            // white has x msec left on the clock
            is >> auxi;
            if (side == WHITE)
                limits.time[WHITE] = auxi;
        }
        else if (token == "btime")
        {
            // black has x msec left on the clock
            is >> auxi;
            if (side == BLACK)
                limits.time[BLACK] = auxi;
        }
        else if (token == "winc")
        {
            // white increment per move in mseconds
            is >> auxi;
            if (side == WHITE)
                limits.incr[WHITE] = auxi;
        }
        else if (token == "binc")
        {
            // black increment per move in mseconds
            is >> auxi;
            if (side == BLACK)
                limits.incr[BLACK] = auxi;
        }
        else if (token == "movestogo")
        {
            // there are x moves to the next time control
            is >> limits.movestogo;
        }
        else if (token == "depth")
        {
            // search x plies only.
            is >> limits.depth;
        }
        else if (token == "nodes")
        {
            // search x nodes max.
            is >> limits.nodes;
        }
        else if (token == "movetime")
        {
            // search exactly x mseconds
            uint64_t searchTime;
            is >> searchTime;
            limits.movetime = searchTime;
        }
    }
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
void Timer::setup(Color color, uint16_t gameClock)
{
    //        std::cout << "time_left   " << limits.time[WHITE] << "  " << limits.time[BLACK] << std::endl;
    //        std::cout << "increment   " << limits.incr[WHITE] << "  " << limits.incr[BLACK] << std::endl;
    //        std::cout << "moves_to_go " << limits.movestogo << std::endl;
    //        std::cout << "depth       " << limits.depth << std::endl;
    //        std::cout << "nodes       " << limits.nodes << std::endl;
    //        std::cout << "move_time   " << limits.movetime << std::endl;
    //        std::cout << "infinite    " << limits.infinite << std::endl;

    searchDepth     = MAX_PLY;
    timeForThisMove = MAX_TIME;

    if (limits.infinite)                     // recherche infinie (temps et profondeur)
    {
        searchDepth     = MAX_PLY;
        timeForThisMove = MAX_TIME;
    }
    else if (limits.depth != 0)             // profondeur de recherche imposée = depth
    {
        searchDepth     = limits.depth;
        timeForThisMove = MAX_TIME;
    }
    else if (limits.movetime != 0)         // temps de recherche imposé = move_time
    {
        searchDepth     = MAX_PLY;
        timeForThisMove = limits.movetime;
    }
    else if (limits.time[color] != 0)
    {
        int time_left = limits.time[color];
        int increment = limits.incr[color];
        int movestogo = limits.movestogo;

        int percent; // How many percent of the time we will use; percent=20 -> 5%, percent=40 -> 2.5% etc. (formula is 100/percent, i.e. 100/40 =2.5)

        // partie : 40 coups en 15 minutes              : moves_to_go = 40 ; wtime=btime = 15*60000 ; winc=binc = 0
        // partie en 5 minutes, incrément de 6 secondes : moves_to_go = 0  ; wtime=btime =  5*60000 ; winc=binc = 6 >> sudden death

        // Idée de Mediocre

        if (movestogo == 0)
        {
            percent = 40; // 2.5% of the time left

            // Use the percent + increment for the move
            timeForThisMove = time_left/percent + increment;

            // If the increment puts us above the total time left
            // use the timeleft - 0.5 seconds
            if(timeForThisMove >= time_left)
                timeForThisMove = time_left - 500;

            // If 0.5 seconds puts us below 0
            // use 0.1 seconds
            if(timeForThisMove < 0)
                timeForThisMove = 100;
        }
    }

    // print debug info
    //        std::cout << "timeForThisMove: " << timeForThisMove <<
    //                     " searchDepth: " << searchDepth << std::endl;
}

//==============================================================================
//! \brief Check if the time we alloted for picking this move has expired.
//------------------------------------------------------------------------------
bool Timer::checkLimits()
{
    auto fin     = std::chrono::high_resolution_clock::now();
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
    auto fin     = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(fin - startTime).count();

    //    std::cout << "finish : e=" << elapsed << " ; t=" << timeForThisMove << std::endl;
    // If we used 90% of the time so far, we break here
    if(elapsed > timeForThisMove*0.9 )
        return true;

    return false;
}

//==================================================================
//! \brief Returns the current time of the system in milliseconds.
//------------------------------------------------------------------
U64 Timer::get_current_time_in_milliseconds()
{
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    U64  ms  = static_cast<I64>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());

    return ms;
}

//==================================================================
//! \brief Affiche le temps passé en millisecondes
//------------------------------------------------------------------
void Timer::show_time()
{
    // Elapsed time in milliseconds
    auto end = std::chrono::high_resolution_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - startTime).count();
    std::cout << "Time           " <<    ms/1000.0 << " s" << std::endl;
}

