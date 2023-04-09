#include "ThreadPool.h"
#include "Board.h"
#include "Search.h"
#include "PolyBook.h"

extern PolyBook Book;

//=================================================
//! \brief  Constructeur défaut
//-------------------------------------------------
ThreadPool::ThreadPool() :
    nbr_threads(1),
    use_book(true),
    log_uci(true)

{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "ThreadPool::constructeur 1 (nbr_threads=%d)", nbr_threads);
    printlog(message);
#endif

}

//=================================================
//! \brief  Constructeur avec arguments
//-------------------------------------------------
ThreadPool::ThreadPool(int m_nbr, bool m_use, bool m_log) :
    nbr_threads(m_nbr),
    use_book(m_use),
    log_uci(m_log)
{
#ifdef DEBUG_LOG
    char message[200];
    sprintf(message, "ThreadPool::constructeur : nbr_threads=%d ; use_book=%d ; log_uci=%d ", nbr_threads, use_book, log_uci);
    printlog(message);
#endif

}

//=================================================
//! \brief  Lance la recherche
//-------------------------------------------------
void ThreadPool::start_thinking(const Board& board, const Timer& timer)
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "ThreadPool::start_thinking");
    printlog(message);
#endif

    MOVE best = 0;
    if(use_book == true && (best = Book.get_move(board)) != 0)
    {
        std::cout << "bestmove " << Move::name(best) << std::endl;
    }
    else
    {
#ifdef DEBUG_LOG
        sprintf(message, "ThreadPool::start_thinking ; lancement de %d threads", nbr_threads);
        printlog(message);
#endif

        int side = board.side_to_move;

        for (int i = 0; i < MAX_THREADS; i++)
        {
            if (threads[i].search)
            {
                delete threads[i].search;
                threads[i].search = nullptr;
            }
        }

        for (int i = 0; i < nbr_threads; i++)
        {
            // copie des arguments
            Board b = board;
            Timer t = timer;
            OrderingInfo o = OrderingInfo();
            threads[i].search = new Search(b, t, o, log_uci, i);
        }

        // Il faut mettre le lancement des threads dans une boucle séparée
        // car il faut être sur que la Search soit bien créée
        for (int i = 0; i < nbr_threads; i++)
        {
            if (side == WHITE)
                threads[i].thread = std::thread(&Search::iterDeep<WHITE>, threads[i].search);
            else
                threads[i].thread = std::thread(&Search::iterDeep<BLACK>, threads[i].search);
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

    for (int i = 1; i < nbr_threads; i++) {
        if (threads[i].search != nullptr) {
            threads[i].search->stop();
        }
    }

    // NE PAS détruire les search, on en a besoin
    // pour calculer le nombre de nodes
}

//=================================================
//! \brief  On impose l'arrêt de la recherche
//! que ce soit une commande uci-stop, ou autre.
//-------------------------------------------------
void ThreadPool::wait(int start)
{
    for (int i = start; i < nbr_threads; i++)
    {
        if (threads[i].thread.joinable())
            threads[i].thread.join();
    }
}

//=================================================
//! \brief  On impose l'arrêt de la recherche
//! que ce soit une commande uci-stop, ou autre.
//-------------------------------------------------
void ThreadPool::stop()
{
    if (threads[0].search)
        threads[0].search->stop();
    if (threads[0].thread.joinable())
        threads[0].thread.join();
}

//=================================================
//! \brief  Sortie du programme
//-------------------------------------------------
void ThreadPool::quit()
{
    stop();

    for (int i = 0; i < nbr_threads; i++)
    {
        delete threads[i].search;
        threads[i].search = nullptr;
    }
}

//=================================================
//! \brief  Retourne le nombre total des nodes recherchés
//-------------------------------------------------
U64 ThreadPool::get_nodes()
{
    U64 total = 0;
    for (int i=0; i<nbr_threads; i++)
    {
        total += threads[i].search->nodes;
    }
    return(total);
}

//=================================================
//! \brief  Retourne le nombre total des nodes recherchés
//-------------------------------------------------
int ThreadPool::get_depths()
{
    int total = 0;
    for (int i=0; i<nbr_threads; i++)
    {
        total += threads[i].search->current_depth;
    }
    return(total);
}

//=================================================
//! \brief  Retourne le meilleur coup trouvé
//-------------------------------------------------
MOVE ThreadPool::get_best()
{
    return threads[0].search->get_best();
}
