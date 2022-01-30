#ifndef TIMER_H
#define TIMER_H

//  Depuis le code Drofa

class Timer;

#include <chrono>
#include "defines.h"
#include "Position.h"

const int DEFAULT_SEARCH_DEPTH = 15;
const int MAX_SEARCH_DEPTH = MAX_PLY; // 64;

class Timer
{
public:

    /**
     * @brief Represents limits imposed on a search through the UCI protocol.
     */
    struct Limits {
        I32  depth;         // Maximum depth to search to
        bool infinite;      // If true, don't stop the search until the stop flag is set
        U64  max_nodes;     // Maximum number of nodes to search
        int  moves_to_go;   // Moves left to the next time control
        int  move_time;     // If nonzero, search exactly this number of milliseconds
        int  time_left;     // time left on the clock
        int  increment;     // increment per move

        Limits() : depth(0), infinite(false), max_nodes(0), moves_to_go(0), move_time(0), time_left(0), increment(0) {};
    };


private:
    /**
     * @brief Limits object representing limits imposed on this search.
     *
     */
    Limits _limits;

    /**
     * @brief Time allocated for this search in ms
     *
     */
    int _timeAllocated;

    /**
     * @brief This variable holds value of how much time left on our
     * clock. If it is too low, we do not prolong search.
     *
     */
    int _ourTimeLeft;

    /**
     * @brief We keep track of times we prolonged thought
     * during the search. It is important to not prolong a more
     * than one in a row in order not to lose on time.
     *
     */
    bool _wasThoughtProlonged;

    /**
     *  @brief We track how much time we spended while
     *  searching last ply. It is used to estimate how much time
     *  we grant engine when search be prolonged.
     */
    int _lastPlyTime;

    /**
     * @brief Depth of this search in plys
     */
    int _searchDepth;

    /**
     * @brief time_point object representing the exact moment this search was started.
     *
     */
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;

    void setupTimer(Position* position);

    //-------------------------------

  public:
        Timer();
        Timer(Limits, Color, int);

      bool checkLimits(U64);
      void startIteration();
      bool finishOnThisDepth(int &, U64, U64);
      int  getSearchDepth() const { return(_searchDepth); }
      void setSearchLimitDepth(int d)   { _limits.depth     = d;    }
      void setSearchLimitTime(int t)    { _limits.move_time = t;    }
      void setSearchInfinite(bool i)    { _limits.infinite  = i;    }

      void reset();
      void go(std::istringstream &is, Position* position);
      void init(Position* position);
      I64  get_current_time_in_milliseconds();
      void clear();
      void show_time();


};

#endif // TIMER_H
