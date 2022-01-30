#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>
#include <cmath>
#include <cstring>

#include "Timer.h"
#include "Position.h"

// https://www.delftstack.com/fr/howto/cpp/how-to-get-time-in-milliseconds-cpp/

//  Code provenant de Drofa

Timer::Timer()
{
#ifdef DEBUG_CLASS
    std::cout << "Timer::Timer" << std::endl;
#endif
    reset();
}

void Timer::reset()
{
#ifdef DEBUG_CLASS
    std::cout << "Timer::reset" << std::endl;
#endif
    _timeAllocated           = 0;   // Time allocated for this search in ms
    _ourTimeLeft             = 0;     //
    _lastPlyTime             = 0;
    _searchDepth             = 0;
    _wasThoughtProlonged     = false;
}

// go depth 6 wtime 180000 btime 100000 binc 1000 winc 1000 movetime 1000 movestogo 40

void Timer::go(std::istringstream &is, Position* position)
{
#ifdef DEBUG_CLASS
    std::cout << "Timer::go" << std::endl;
#endif
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
            _limits.infinite = true;
        }

        else if (token == "binc")
        {
            // black increment per move in mseconds
            is >> auxi;
            if (position->side_to_move == BLACK)
                _limits.increment = auxi;
        }

        else if (token == "winc")
        {
            // white increment per move in mseconds
            is >> auxi;
            if (position->side_to_move == WHITE)
                _limits.increment = auxi;
        }

        else if (token == "wtime")
        {
            // white has x msec left on the clock
            is >> auxi;
            if (position->side_to_move == WHITE)
                _limits.time_left = auxi;
        }

        else if (token == "btime")
        {
            // black has x msec left on the clock
            is >> auxi;
            if (position->side_to_move == BLACK)
                _limits.time_left = auxi;
        }

        else if (token == "movestogo")
        {
            // there are x moves to the next time control
            is >> _limits.moves_to_go;
        }

        else if (token == "movetime")
        {
            // search exactly x mseconds
            uint64_t searchTime;
            is >> searchTime;
            _limits.move_time = searchTime;
        }

        else if (token == "depth")
        {
            // search x plies only.
            is >> _limits.depth;
        }

        else if (token == "nodes")
        {
            // search x nodes max.
            is >> _limits.max_nodes;
        }
    }

    //------------------------------------

    init(position);

}

//=======================================================================
//! \brief  Initialisation des limites en temps pour la recherche
//!
//! \param[in] position     position de recherche
//-----------------------------------------------------------------------
void Timer::init(Position* position)
{
#ifdef DEBUG_CLASS
    std::cout << "Timer::init" << std::endl;
#endif
    // Code provenant de Drofa

    //   std::cout << "depth " << limits.depth << std::endl;
    //   std::cout << "infinite " << limits.infinite << std::endl;
    //   std::cout << "moves_to_go " << limits.moves_to_go << std::endl;
    //   std::cout << "move_time " << limits.move_time << std::endl;
    //   std::cout << "time_left " << limits.time_left << std::endl;
    //   std::cout << "increment " << limits.increment << std::endl;

    _wasThoughtProlonged = false;

    if (_limits.infinite)                     // recherche infinie (temps et profondeur)
    {
        _searchDepth   = MAX_PLY;
        _timeAllocated = MAX_TIME;
    }
    else if (_limits.depth != 0)             // profondeur de recherche imposée = depth
    {
        _searchDepth   = _limits.depth;
        _timeAllocated = MAX_TIME;
    }
    else if (_limits.move_time != 0)         // temps de recherche imposé = move_time
    {
        _searchDepth   = MAX_SEARCH_DEPTH;
        _timeAllocated = _limits.move_time;
    }
    else if (_limits.time_left != 0)         // il reste 'time_left' à la pendule
    {
        setupTimer(position);
    }
    else                                    // No limits specified, use default depth
    {
        _searchDepth   = DEFAULT_SEARCH_DEPTH;
        _timeAllocated = MAX_TIME;
    }


    // print debug info
//    std::cout << "time: " << _timeAllocated <<
//                 " depth: " << _searchDepth << std::endl;
}

void Timer::setupTimer(Position* position)
{
#ifdef DEBUG_CLASS
    std::cout << "Timer::setupTimer" << std::endl;
#endif

    int ourTime = _limits.time_left;
     //int opponentTime = limits.time_left[_initialBoard.getInactivePlayer()];
  //   int moveNum = board._getGameClock() / 2;
     int moveNum = position->ply / 2;
     int ourIncrement = _limits.increment;

     int tWidth_a = 30;
     int tWidth = 175;
     int tMove = 20;
     int criticalMove = 28;

     double tCoefficient = 0;

     // Divide up the remaining time (If movestogo not specified we are in
     // sudden death)
     if (_limits.moves_to_go == 0)
     {
         std::cout << "moveNum " << moveNum << std::endl;
         tCoefficient = 10 * (tWidth_a / pow((tWidth + pow((moveNum - tMove), 2)), 1.5));
         _timeAllocated = ourTime * tCoefficient;
         if (moveNum > criticalMove)
             _timeAllocated = ourTime / 10 + ourIncrement;
         _timeAllocated = std::min(_timeAllocated, ourTime + ourIncrement - 10);
     }
     else
     {
         // when movetogo is specified, use different coefficients
         tWidth_a = 75;
         tWidth = 200;
         tMove = 35;
         criticalMove = 20;

         tCoefficient = 10 * (tWidth_a / pow((tWidth + pow((moveNum - tMove), 2)), 1.5));
         _timeAllocated = ourTime * tCoefficient;
         if (moveNum > criticalMove)
             _timeAllocated = ourTime / 10 + ourIncrement;
         _timeAllocated = std::min(_timeAllocated, ourTime + ourIncrement - 10);
     }

     // Depth is infinity in a timed search (ends when time runs out)
     _searchDepth = MAX_SEARCH_DEPTH;
     _ourTimeLeft = ourTime - _timeAllocated;
}

/*
 * Returns the current time of the system
 * in milliseconds.
 */
I64 Timer::get_current_time_in_milliseconds()
{
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    I64  ms  = static_cast<I64>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());

    return ms;
}

/**
 * @brief Returns True if this search has exceeded its given limits
 *
 * Note that to avoid a needless amount of computation, limits are only
 * checked every 4096 calls to _checkLimits() (using the Search::_limitCheckCount property).
 * If Search::_limitCheckCount is not 0, false will be returned.
 *
 * @return True if this search has exceed its limits, true otherwise
 */
bool Timer::checkLimits(U64 nodes)
{
    auto fin     = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(fin - _start).count();

    if (_limits.max_nodes != 0 && (nodes >= _limits.max_nodes))
        return true;

    // when searching at a time control we will try to use time efficiatnly.
    // If we already started the search, but it took way longer than expected
    // we actually do not want to lose all of our effort
    // So we check if we have enough time to actually finish it
    // If we have much time left, we will allocate some more
    // time to finish search and set a flag that search was prolonged
    // so we didnt prolong it again.

    if (_wasThoughtProlonged && elapsed >= (_timeAllocated))
    {
        return true;
    }
    else  if (elapsed >= (_timeAllocated))
    {
        // if we have so much time left that we supposedly
        // can search last ply ~25 times at least
        // we can prolong thought here.
        if (_ourTimeLeft > _lastPlyTime * 20 + 30 )
        {
            _timeAllocated += _lastPlyTime * 2;
            _wasThoughtProlonged = true;
            return false;
        }
        else
        {
            return true;
        }

    }

    return false;

}

void Timer::startIteration()
{
    _start = std::chrono::high_resolution_clock::now();
    _lastPlyTime = 0;
}

//===========================================================
//! \brief  Détermine si on a assez de temps pour effectuer
//!         une nouvelle itération
//-----------------------------------------------------------
bool Timer::finishOnThisDepth(int& elapsedTime, U64 totalNodes, U64 bestNodes)
{
    auto fin     = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(fin - _start).count();

    _lastPlyTime =  elapsed - _lastPlyTime;


//    double nodesConfidance = bestNodes * 100.0 / totalNodes;
//    // clamp coeff between 25 and 75
//    // we assume that standart case is about ~50% of nodes go in bestMove
//    nodesConfidance = std::max(25.0, nodesConfidance);
//    nodesConfidance = std::min(85.0, nodesConfidance);

//    double nodesConfidance = 50.0;

    // J'ai simplifié ici, et ça marche bien pour moi.

    double nodesCoeff = 1.0 ; // + (50.0 - nodesConfidance) / 50.0;

    elapsedTime = elapsed;
    if (_wasThoughtProlonged ||  (elapsed >= (_timeAllocated * nodesCoeff * 0.5)))
        return true;
    else
        return false;


}


void Timer::show_time()
{
    // Elapsed time in milliseconds
    auto end = std::chrono::high_resolution_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - _start).count();
    std::cout << "Time           " <<    ms/1000.0 << " s" << std::endl;
}

