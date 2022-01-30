#include <iostream>
#include <sstream>
#include <string>
#include "defines.h"
#include "Search.h"

// position fen fenstr
// position startpos
// ... moves e2e4 e7e5 b7b8q
void Search::position(std::istringstream &is)
{
#ifdef DEBUG_CLASS
    std::cout << "Search::position"<< std::endl;
#endif

    std::string token;

    token.clear();
    is >> token;

    if (token == "startpos")
    {
        init_fen(START_FEN);
    }
    else
    {
        std::string fen;

        while (is >> token && token != "moves")
        {
            fen += token + " ";
        }

        init_fen(fen);
    }

    while (is >> token)
    {
        if (token == "moves")
        {
            continue;
        }

        //       display_ascii();
        gen_moves();

        for (int index = first_move[positions->ply]; index < first_move[positions->ply + 1]; ++index)
        {
            if (moves[index].show(output) == token)
            {
                make_move(&moves[index]);

                positions->ply = 0;

                break;
            }
        }
    }

 //   display_ascii();
}

//-----------------------------------------------------
//! \brief Commande UCI : go
//-----------------------------------------------------
void Search::go(std::istringstream &is)
{
#ifdef DEBUG_CLASS
    std::cout << "Search::go" << std::endl;
#endif

    // reset time control
    timer.reset();

    // parse uci
    timer.go(is, positions);

    // search
    think();
}

//-----------------------------------------------------
//! \brief Commande UCI : stop
//-----------------------------------------------------
void Search::stop()
{
#ifdef DEBUG_CLASS
    std::cout << "Search::stop" << std::endl;
#endif

    stopped = true;
}

//-----------------------------------------------------
//! \brief Commande UCI : newgame
//!         Initialisation au début d'une recherche
//!        lors d'une nouvelle d'une partie
//-----------------------------------------------------
void Search::new_game()
{
#ifdef DEBUG_CLASS
    std::cout << "Search::new_game" << std::endl;
#endif

    stop();
    new_search();
    hashtable.clear();
    positions->reset(); //TODO à faire ?
}
