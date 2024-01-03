#ifndef THREADPOOL_H
#define THREADPOOL_H

class ThreadPool;

#include "defines.h"
#include "Board.h"
#include "Timer.h"
#include "Search.h"

class ThreadPool
{
public:
    explicit ThreadPool(int _nbr, bool _tb, bool _log);
    void set_threads(int nbr);
    void create();
    void init();
    void reset();

    void start_thinking(const Board &board, const Timer &timer);
    void main_thread_stopped();
    void stop();
    void wait(int start);
    void quit();

    U64  get_all_nodes() const;
    int  get_all_depths() const;
    MOVE get_best_move() const { return threadData[0].best_move; }
    U64  get_all_tbhits() const;

    void set_logUci(bool f)     { logUci = f;       }
    void set_useSyzygy(bool f)  { useSyzygy = f;    }

    bool get_logUci() const { return logUci; }
    int  get_nbrThreads() const { return nbrThreads; }
    bool get_useSyzygy() const { return useSyzygy; }

    std::array<ThreadData, MAX_THREADS> threadData;

private:
    int     nbrThreads;
    bool    useSyzygy;
    bool    logUci;

};

extern ThreadPool threadPool;

#endif // THREADPOOL_H
