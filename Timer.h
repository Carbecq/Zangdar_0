#ifndef TIMER_H
#define TIMER_H


class Timer;

#include <cstdint>
#include <chrono>
#include "defines.h"
#include "color.h"

struct Limits {

    int  time[2];     // time left for black and white
    int  incr[2];     // increment for black and white
    int  movestogo;   // (for cyclic) - moves till next time addition
    int  depth;       // limit search by depth
    U64  nodes;       // limit search by nodes searched
    int  movetime;    // limit search by time
    bool infinite;    // ignore limits (infinite search)

    Limits() : time{}, incr{}, movestogo(0), depth(0), nodes(0), movetime(0), infinite(false) {};
};

class Timer
{
public:
    Timer();
    Timer(bool infinite, int wtime, int btime, int winc, int binc, int movestogo,
          int depth, int nodes, int movetime);

    Limits limits;

    void reset();
    void go(std::istringstream &is, Color side);
    U64  get_current_time_in_milliseconds();
    void clear();
    void show_time();

    void start();
    void setup(Color color, uint16_t gameClock);
    bool checkLimits();
    bool finishOnThisDepth(U64& elapsed);
    int  getSearchDepth() const { return(searchDepth); }
    void setSearchInfinite(bool f) {limits.infinite = f;}
    void setSearchLimitDepth(int d) {limits.depth = d;}
    void setSearchLimitTime(int t) {limits.movetime = t;}

private:

    // gives the exact moment this search was started.
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    U64 timeForThisMove = 0;   // temps en ms
    int searchDepth;


};

#endif // TIMER_H
