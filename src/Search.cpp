#include "Search.h"
#include "ThreadPool.h"
#include "move.h"
#include <iomanip>
#include <locale>
#include <thread>

extern ThreadPool threadPool;

//=============================================
//! \brief  Constructeur
//---------------------------------------------
Search::Search(const Board &m_board, const Timer &m_timer, OrderingInfo &m_info, bool m_log, int m_id)
    : nodes(0)
    , stopped(false)
    , threadID(m_id)
    , my_board(m_board)
    , my_timer(m_timer)
    , output(4)
    , best_move(0)
    , logUci(m_log)
    , my_orderingInfo(m_info)
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "Search constructeur");
    printlog(message);
#endif
}

//=============================================
//! \brief  Destructeur
//---------------------------------------------
Search::~Search() {}

//======================================================
//! \brief Initialisation au début d'une recherche
//!        dans le cadre d'une partie
//! ATTENTION : on ne remet pas à zéro la table de transposition
//------------------------------------------------------
void Search::new_search()
{
    stopped = false;
    nodes = 0;

    // game_clock n'est pas modifié, car on est toujours
    // dans la partie
}

//=========================================================
//! \brief  Affichage UCI du résultat de la recherche
//!
//! \param[in] depth        profondeur de la recherche
//! \param[in] best_score   meilleur score
//! \param[in] elapsed      temps passé pour la recherche, en millisecondes
//---------------------------------------------------------
void Search::show_uci_result(int depth, int best_score, U64 elapsed, MOVE *pv) const
{
    elapsed++; // évite une division par 0
    // commande envoyée à UCI
    // voir le document : uci_commands.txt

    //plante        std::cout.imbue(std::locale("fr"));

    // en mode UCI, il ne faut pas mettre les points de séparateur,
    // sinon Arena, n'affiche pas correctement le nombre de nodes
#ifdef PRETTY
    std::cout.imbue(std::locale(std::locale::classic(), new MyNumPunct));
    int l = 12;
#else
    int l = 10;
#endif

    std::cout << "info ";

    /*
     * score
     *       cp <x>
     *           the score from the engine's point of view in centipawns.
     *       mate <y>
     *           mate in y moves, not plies.
     *           If the engine is getting mated use negative values for y.
     */

    if (best_score >= MAX_EVAL) {
        std::cout << "mate " << std::setw(2) << (MATE - best_score) / 2 + 1;
        std::cout << "      ";
    } else if (best_score <= -MAX_EVAL) {
        std::cout << "mate " << std::setw(2) << (-MATE - best_score) / 2;
        std::cout << "      ";
    } else {
        //collect info about nodes from all Threads
        U64 all_nodes = threadPool.get_nodes();

        // nodes    : noeuds calculés
        // nps      : nodes per second searched
        // time     : the time searched in ms

#ifdef PRETTY
        std::cout << "score cp " << std::right << std::setw(4) << best_score; // the score from the engine's point of view in centipawns
        std::cout << " depth " << std::setw(2) << depth
                  //             << " seldepth " << std::setw(2) << seldepth
                  << " nodes " << std::setw(l) << all_nodes
                  << " nps " << std::setw(7) << all_nodes * 1000 / elapsed
                  << " time " << std::setw(6) << elapsed;
#else
        std::cout << "score cp "
                  << best_score; // the score from the engine's point of view in centipawns

        std::cout << " depth "
                  << depth
                  //             << " seldepth " << std::setw(2) << seldepth
                  << " nodes " << all_nodes << " nps " << all_nodes * 1000 / elapsed << " time "
                  << elapsed;

#endif
    }

    std::cout << " pv";
    for (MOVE *p = pv; *p != 0; p++)
        std::cout << " " << Move::name(*p);

    std::cout << std::endl;
}

//=========================================================
//! \brief  Affichage UCI du meilleur coup trouvé
//!
//! \param[in]  name   coup en notation UCI
//---------------------------------------------------------
void Search::show_uci_best(const MOVE best_move) const
{
    // ATTENTION AU FORMAT D'AFFICHAGE
    std::cout << "bestmove " << Move::name(best_move) << std::endl;
}

//=========================================================
//! \brief  Mise à jour de la Principal variation
//!
//! \param[in]  name   coup en notation UCI
//---------------------------------------------------------
void Search::update_pv(MOVE *dst, MOVE *src, MOVE move) const
{
    *dst++ = move;
    while ((*dst++ = *src++))
        ;
}

//=========================================================
//! \brief  Controle du time-out
//! \return Retourne "true" si la recherche a dépassé sa limite de temps
//!
//! De façon à éviter un nombre important de calculs , on ne fera
//! ce calcul que tous les 4096 coups.
//---------------------------------------------------------
bool Search::check_limits()
{
    // Every 4096 nodes, check if our time has expired.
    if ((nodes & 4095) == 0)
        return my_timer.checkLimits();
    return false;
}
