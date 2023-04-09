#ifndef THREADPOOL_H
#define THREADPOOL_H

class ThreadPool;

#include <thread>
#include "defines.h"
#include "Board.h"
#include "Timer.h"
#include "Search.h"

struct Thread {
    std::thread thread;
    Search*     search;
};


class ThreadPool
{
public:
    ThreadPool();
    ThreadPool(int m_nbr, bool m_use, bool m_log);
    void start_thinking(const Board& board, const Timer& timer);
    void main_thread_stopped();
    void stop();
    void wait(int start);
    void quit();

    U64  get_nodes();
    int  get_depths();
    MOVE get_best();

    void set_nbrThreads(int n)  { nbr_threads = n;  }
    void set_useBook(bool f)    { use_book = f;     }
    void set_logUci(bool f)     { log_uci = f;      }

    int  get_nbrThreads()   const { return(nbr_threads);  }
    bool get_useBook()      const { return(use_book);     }

    std::array<Thread, MAX_THREADS> threads;

private:
    int     nbr_threads;
    bool    use_book;
    bool    log_uci;

};

extern ThreadPool threadPool;

#endif // THREADPOOL_H
