#include "Search.h"
#include "ThreadPool.h"
#include "Move.h"
#include <iomanip>
#include <locale>
#include <thread>
#include <cmath>

extern ThreadPool threadPool;

//=============================================
//! \brief  Constructeur
//---------------------------------------------
Search::Search(const Board &m_board, const Timer &m_timer, OrderingInfo &m_info, bool m_log)
    : stopped(false)
    , my_board(m_board)
    , my_timer(m_timer)
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

//=========================================================
//! \brief  Affichage UCI du résultat de la recherche
//!
//! \param[in] depth        profondeur de la recherche
//! \param[in] best_score   meilleur score
//! \param[in] elapsed      temps passé pour la recherche, en millisecondes
//---------------------------------------------------------
void Search::show_uci_result(const ThreadData* td, U64 elapsed, PVariation& pv) const
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
    
    if (td->best_score >= MATE_IN_X) {
        std::cout << "mate " << std::setw(2) << (MATE - td->best_score) / 2 + 1;
        std::cout << "      ";
        printf(" plus ply=%d  ", MATE-td->best_score);
    } else if (td->best_score <= -MATE_IN_X) {
        std::cout << "mate " << std::setw(2) << (-MATE - td->best_score) / 2;
        std::cout << "      ";
        printf(" moins ply=%d  ", -MATE-td->best_score);
    } else {
        //collect info about nodes from all Threads
        U64 all_nodes = threadPool.get_all_nodes();

        // nodes    : noeuds calculés
        // nps      : nodes per second searched
        // time     : the time searched in ms

#ifdef PRETTY
        std::cout << "score cp " << std::right << std::setw(4) << td->best_score; // the score from the engine's point of view in centipawns
        std::cout << " depth " << std::setw(2) << td->best_depth
                  //             << " seldepth " << std::setw(2) << seldepth
                  << " nodes " << std::setw(l) << all_nodes
                  << " nps " << std::setw(7) << all_nodes * 1000 / elapsed
                  << " time " << std::setw(6) << elapsed;
#else
        std::cout << "score cp "
                  << td->best_score; // the score from the engine's point of view in centipawns

        std::cout << " depth "
                  << td->best_depth
                  //             << " seldepth " << std::setw(2) << seldepth
                  << " nodes " << all_nodes << " nps " << all_nodes * 1000 / elapsed << " time "
                  << elapsed;

#endif
    }

    std::cout << " pv";
    for (int i=0; i<pv.length; i++)
        std::cout << " " << Move::name(pv.line[i]);

    std::cout << std::endl;
}

//=========================================================
//! \brief  Affichage UCI du meilleur coup trouvé
//!
//! \param[in]  name   coup en notation UCI
//---------------------------------------------------------
void Search::show_uci_best(const ThreadData* td) const
{
    // ATTENTION AU FORMAT D'AFFICHAGE
    std::cout << "bestmove " << Move::name(td->best_move) << std::endl;
}

//=========================================================
//! \brief  Affichage UCI du coup en cours d'étude
//!
//---------------------------------------------------------
void Search::show_uci_current(MOVE move, int currmove, int depth) const
{
    std::cout << "info depth "      << depth
              << " currmove "       << Move::show(move, 4)
              << " currmovenumber " << currmove
              << std::endl;
}

//=========================================================
//! \brief  Mise à jour de la Principal variation
//!
//! \param[in]  name   coup en notation UCI
//---------------------------------------------------------
void Search::update_pv(PVariation& pv, const PVariation& new_pv, const MOVE move) const
{
    pv.length  = 1 + new_pv.length;
    pv.line[0] = move;
    memcpy(pv.line + 1, new_pv.line, sizeof(MOVE) * new_pv.length);
}

//=========================================================
//! \brief  Controle du time-out
//! \return Retourne "true" si la recherche a dépassé sa limite de temps
//!
//! De façon à éviter un nombre important de calculs , on ne fera
//! ce calcul que tous les 4096 coups.
//---------------------------------------------------------
bool Search::check_limits(const ThreadData* td) const
{
    // Every 4096 nodes, check if our time has expired.
    // On ne teste pas si nodes=0
    if ((td->nodes & 4095) == 4095)
        return my_timer.checkLimits();
    else
        return false;
}

//-----------------------------------------------------
//! \brief Commande UCI : stop
//-----------------------------------------------------
void Search::stop()
{
    //    printf(">>>>>>> SEARCH STOP recue \n");fflush(stdout);

    stopped = true;
}
