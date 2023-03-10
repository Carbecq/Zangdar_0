#include <iostream>
#include <sstream>
#include <string>
#include "Search.h"
#include "defines.h"


//-----------------------------------------------------
//! \brief Commande UCI : newgame
//!         Initialisation au début d'une recherche
//!        lors d'une nouvelle d'une partie
//-----------------------------------------------------
void Search::new_game()
{
    stop();
    new_search();
    transtable.clear();
    //    positions->reset(); //TODO à faire ?

}

/* http://talkchess.com/forum3/viewtopic.php?f=7&t=64262&p=718805&hilit=terminating+process#p718805
 * http://talkchess.com/forum3/viewtopic.php?f=7&t=56303&hilit=poll+input&sid=2fce7f7963765e46d2cbb8146b371226
 * https://www.chessprogramming.org/CPW-Engine_com
 * http://talkchess.com/forum3/viewtopic.php?f=7&t=54703&p=601188&hilit=poll+input&sid=19e50722c3bb11888f1a8f8aa8fafef8#p601188
 *
I always poll for input, and/or completion time from the Search routine. When the input cannot be processed during search, I leave it in the input buffer. The ReadLine() function used in the main loop first checks if there is something in the input buffer, and only reads a line from stdin if there wasn't. After that it processes the line.

Note that in UCI you must ignore commands that are not supposed to come. When searching you are basically waiting for 'quit', 'stop', 'ponderhit', 'isready' or 'debug'. The latter three can be handled in search, while 'stop' should terminate the search, and 'quit' the process. Other commands are not legal, and thus should be ignored. You should not try to postpone their processing until after receiving 'stop'. *
 */

//-----------------------------------------------------
//! \brief Commande UCI : position
//!         Entrée d'une position
//!         et d'une série de coups
//-----------------------------------------------------
void Board::position(std::istringstream &is)
{
 //   printf("Board::position \n");fflush(stdout);
    std::string token;

    token.clear();
    is >> token;
//ZZ
    bool logTactics = false;

    if (token == "startpos")
    {
        set_fen(START_FEN, logTactics);
    }
    else
    {
        std::string fen;

        while (is >> token && token != "moves")
        {
            fen += token + " ";
        }
        set_fen(fen, logTactics);
    }

    while (is >> token)
    {
        if (token == "moves")
            continue;
        if (side_to_move == WHITE)
            apply_token<WHITE>(token);
        else
            apply_token<BLACK>(token);
    }

}

//-----------------------------------------------------
//! \brief Commande UCI : stop
//-----------------------------------------------------
void Search::stop()
{
//    printf(">>>>>>> SEARCH STOP recue \n");fflush(stdout);

    stopped = true;
}

//-----------------------------------------------------
//! \brief Commande UCI : quit
//-----------------------------------------------------
void Search::quit()
{
    stop();
}

