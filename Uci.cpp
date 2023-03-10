#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <thread>
#include <memory>

#include "defines.h"
#include "Uci.h"
#include "Search.h"
#include "Timer.h"

//  Idées empruntées à Koivisto
// voir Drofa / Shallow_blue

std::thread searchThread;
std::shared_ptr<Search> uci_search;

Board      uci_board;
Timer      uci_timer;


void goThink(int side, Board* m_board, Timer* m_timer)
{
    if (side == WHITE)
        uci_search->think<WHITE>(m_board, m_timer);
    else
        uci_search->think<BLACK>(m_board, m_timer);
}

//======================================
//! \brief  Boucle principale UCI
//--------------------------------------
void Uci::run()
{
    std::string line;

    uci_search = std::make_shared<Search>();

    // print engine info
    std::cout << "id name Zangdar " << VERSION << std::endl;
    std::cout << "id author Philippe Chevalier" << std::endl;
    std::cout << "uciok" << std::endl;

    // main loop
    processCommand();

}

//==============================================
//! \brief
//----------------------------------------------
void Uci::processCommand()
{
    std::string token;
    std::string line;

    while (getline(std::cin, line))
    {
        std::istringstream is(line);

        token.clear();
        is >> std::skipws >> token;

        //-------------------------------------------- gui -> engine

        if (token == "uci")
        {
            /* "uciok" has to appear quickly after the GUI sent "uci",
             * and lengthy init stuff is not allowed.
             */


            // print engine info
            std::cout << "id name Zangdar " << VERSION << std::endl;
            std::cout << "id author Philippe Chevalier" << std::endl;
            std::cout << "uciok" << std::endl;
        }

        else if (token == "isready")
        {
            /* "isready" is meant as ping/pong replacement AND feature done with init,
             * and the UCI spec explicitely mentions tablebase initialistion as example
             * that may take some time.
             * The engine only responds with "readyok" after it has processed the setoptions commands.
             * The case where "isready" must be answered immediately is only after init
             * and during search because then there is no reason why the engine should be hanging.
             */

            // synchronize the engine with the GUI
            std::cout << "readyok" << std::endl;
        }

        else if (token == "position")
        {
            // set up the position described in fenstring
            uci_board.position(is);
        }

        else if (token == "ucinewgame")
        {
            // the next search (started with "position" and "go") will be from
            // a different game.
            uci_search->new_game();
        }

        else if (token == "go")
        {
            go(is);
        }

        else if (token == "quit")
        {
            uci_search->quit();
            // quit the program as soon as possible
            exit(0);
        }

        /* Yes, 'stop' needs to be evaluated while the engine is calculating its move. 'isready' the same.
         */

        else if (token == "stop")
        {
            // stop calculating as soon as possible
            stop();
        }



        //------------------------------------------------------- engine -> gui
    }
}

/**
 * parses the uci command: stop
 * stops the current search. This will usually print a last info string and the best move.
 */
void Uci::stop()
{
    uci_search->stop();
    if (searchThread.joinable()) {
        searchThread.join();
    }
}

void Uci::go(std::istringstream& is)
{
    bool infinite   = false;
    int wtime       = 0;
    int btime       = 0;
    int winc        = 0;
    int binc        = 0;
    int movestogo   = 0;
    int depth       = 0;
    int nodes       = 0;
    int movetime    = 0;

    std::string token;
    token.clear();

    // We received a start command. Extract all parameters from the
    // command and start the search.

    while (is >> token)
    {
        if (token == "infinite")
        {
            // search until the "stop" command. Do not exit the search without being told so in this mode!
            infinite = true;
        }
        else if (token == "wtime")
        {
            // white has x msec left on the clock
            is >> wtime;
        }
        else if (token == "btime")
        {
            // black has x msec left on the clock
            is >> btime;
        }
        else if (token == "winc")
        {
            // white increment per move in mseconds
            is >> winc;
        }
        else if (token == "binc")
        {
            // black increment per move in mseconds
            is >> binc;
        }
        else if (token == "movestogo")
        {
            // there are x moves to the next time control
            is >> movestogo;
        }
        else if (token == "depth")
        {
            // search x plies only.
            is >> depth;
        }
        else if (token == "nodes")
        {
            // search x nodes max.
            is >> nodes;
        }
        else if (token == "movetime")
        {
            // search exactly x mseconds
            uint64_t searchTime;
            is >> searchTime;
            movetime = searchTime;
        }
    }


    uci_timer = Timer(infinite, wtime, btime, winc, binc, movestogo, depth, nodes, movetime);

    int stm = uci_board.side_to_move;
    searchThread = std::thread(goThink, stm, &uci_board, &uci_timer);
    searchThread.detach();
}
