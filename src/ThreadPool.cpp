#include "ThreadPool.h"
#include "Board.h"
#include "Search.h"
#include "PolyBook.h"
#include "TranspositionTable.h"
#include "Move.h"


//=================================================
//! \brief  Constructeur avec arguments
//-------------------------------------------------
ThreadPool::ThreadPool(int _nbr, bool _tb, bool _log) :
    nbrThreads(_nbr),
    useSyzygy(_tb),
    logUci(_log)
{
#if defined DEBUG_LOG
    char message[200];
    sprintf(message, "ThreadPool::constructeur : nbr_threads=%d ; use_book=%d ; log_uci=%d ", nbr_threads, use_book, log_uci);
    printlog(message);
#endif

    set_threads(_nbr);
}

//=================================================
//! \brief  Initialisation du nombre de threads
//-------------------------------------------------
void ThreadPool::set_threads(int nbr)
{
    int processorCount = static_cast<int>(std::thread::hardware_concurrency());
    // Check if the number of processors can be determined
    if (processorCount == 0)
        processorCount = MAX_THREADS;

    // Clamp the number of threads to the number of processors
    nbrThreads     = std::min(nbr, processorCount);
    nbrThreads     = std::max(nbrThreads, 1);
    nbrThreads     = std::min(nbrThreads, MAX_THREADS);

    create();
}

//=================================================
//! \brief  Initialisation des valeurs des threads
//! lors de la création
//-------------------------------------------------
void ThreadPool::create()
{
    for (int i = 0; i < nbrThreads; i++)
    {
        threadData[i].index      = i;
        threadData[i].depth      = 0;
        threadData[i].score      = -INFINITE;
        threadData[i].seldepth   = 0;
        threadData[i].nodes      = 0;
        threadData[i].stopped    = false;
    }
}

//=================================================
//! \brief  Initialisation des valeurs des threads
//! Utilisé lors de "start_thinking"
//-------------------------------------------------
void ThreadPool::init()
{
    for (int i = 0; i < nbrThreads; i++)
    {
        threadData[i].nodes      = 0;
        threadData[i].seldepth   = 0;

        memset(threadData[i].info.eval,     0,               sizeof(threadData[i].info.eval));
    }
}

//=================================================
//! \brief  Initialisation des valeurs des threads
//! Utilisé lors de "newgame"
//-------------------------------------------------
void ThreadPool::reset()
{
    for (int i = 0; i < nbrThreads; i++)
    {
        threadData[i].nodes      = 0;
        threadData[i].seldepth   = 0;

        memset(threadData[i].info.eval,     0, sizeof(threadData[i].info.eval));
        memset(threadData[i].info.killer1, Move::MOVE_NONE, sizeof(threadData[i].info.killer1));
        memset(threadData[i].info.killer2, Move::MOVE_NONE, sizeof(threadData[i].info.killer2));
        memset(threadData[i].info.history,  0, sizeof(threadData[i].info.history));
    }
}

//=================================================
//! \brief  Lance la recherche
//! Fonction lancée par Uci::parse_go
//-------------------------------------------------
void ThreadPool::start_thinking(const Board& board, const Timer& timer)
{
#if defined DEBUG_LOG
    char message[100];
    sprintf(message, "ThreadPool::start_thinking");
    printlog(message);
#endif

    MOVE best = 0;

    // Probe Opening Book
    if(ownBook.get_useBook() == true && (best = ownBook.get_move(board)) != 0)
    {
        std::cout << "bestmove " << Move::name(best) << std::endl;
    }

    // Probe Syzygy TableBases
    else if (useSyzygy && board.probe_root(best) == true)
    {
        std::cout << "bestmove " << Move::name(best) << std::endl;
    }
    else
    {
#if defined DEBUG_LOG
        sprintf(message, "ThreadPool::start_thinking ; lancement de %d threads", nbr_threads);
        printlog(message);
#endif

        create();
        init();

        for (int i=0; i<nbrThreads; i++)
            threadData[i].stopped = false;

        // Préparation des tables de transposition
        transpositionTable.update_age();

        // On utilise la même instance de Search pour toutes les threads,
        // à condition que les threads n'utilisent pas les mêmes valeurs.
        Search uci_search;

        // Il faut mettre le lancement des threads dans une boucle séparée
        // car il faut être sur que la Search soit bien créée

        int side = board.side_to_move;

        for (int i = 0; i < nbrThreads; i++)
        {
            // copie des arguments
            Board b = board;
            Timer t = timer;

            if (side == WHITE)
                threadData[i].thread = std::thread(&Search::think<WHITE>, &uci_search, b, t, i);
            else
                threadData[i].thread = std::thread(&Search::think<BLACK>, &uci_search, b, t, i);
        }
    }
}

//=================================================
//! \brief  Arrêt de la recherche
//! L'arrêt a été commandé par la thread 0
//! lorsqu'elle a fini sa recherche.
//-------------------------------------------------
void ThreadPool::main_thread_stopped()
{
    // envoie à toutes les autres threads
    // le signal d'arrêter

    for (int i = 1; i < nbrThreads; i++)
        threadData[i].stopped = true;

}

//=================================================
//! \brief  On impose l'arrêt de la recherche
//! que ce soit une commande uci-stop, ou autre.
//-------------------------------------------------
void ThreadPool::wait(int start)
{
    for (int i = start; i < nbrThreads; i++)
    {
        if (threadData[i].thread.joinable())
            threadData[i].thread.join();
    }
}

//=================================================
//! \brief  On impose l'arrêt de la recherche
//! que ce soit une commande uci-stop, ou autre.
//-------------------------------------------------
void ThreadPool::stop()
{
    threadData[0].stopped = true;

    if (threadData[0].thread.joinable())
        threadData[0].thread.join();
}

//=================================================
//! \brief  Sortie du programme
//-------------------------------------------------
void ThreadPool::quit()
{
    stop();

}

//=================================================
//! \brief  Retourne le nombre total des nodes recherchés
//-------------------------------------------------
U64 ThreadPool::get_all_nodes() const
{
    U64 total = 0;
    for (int i=0; i<nbrThreads; i++)
    {
        total += threadData[i].nodes;
    }
    return(total);
}

//=================================================
//! \brief  Retourne le nombre total des nodes recherchés
//-------------------------------------------------
int ThreadPool::get_all_depths() const
{
    int total = 0;
    for (int i=0; i<nbrThreads; i++)
    {
        total += threadData[i].best_depth;
    }
    return(total);
}

//=================================================
//! \brief  Retourne le nombre total de tbhits
//-------------------------------------------------
U64 ThreadPool::get_all_tbhits() const
{
    U64 total = 0;
    for (int i=0; i<nbrThreads; i++)
    {
        total += threadData[i].tbhits;
    }
    return(total);
}


