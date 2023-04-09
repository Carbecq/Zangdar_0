#ifndef TIMER_H
#define TIMER_H

// Code inspiré par Blunder

class Timer;

#include <cstdint>
#include <chrono>
#include "defines.h"
#include "color.h"

class Timer
{
public:
    Timer();
    Timer(bool infinite, int wtime, int btime, int winc, int binc, int movestogo,
          int depth, int nodes, int movetime);

    struct Limits {

        int  time[2];     // time left for black and white
        int  incr[2];     // increment for black and white
        int  movestogo;   // moves
        int  depth;       // limit search by depth
        U64  nodes;       // limit search by nodes searched
        int  movetime;    // limit search by time
        bool infinite;    // ignore limits (infinite search)

        Limits() : time{}, incr{}, movestogo(0), depth(0), nodes(0), movetime(0), infinite(false) {};
    };
    Limits limits;

    void reset();
    void show_time();

    void start();
    void setup(Color color);
    bool checkLimits();
    bool finishOnThisDepth(U64& elapsed);
    int  getSearchDepth() const { return(searchDepth); }

private:

    // gives the exact moment this search was started.
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    U64 timeForThisMove = 0;   // temps en ms
    int searchDepth;


};

#endif // TIMER_H
