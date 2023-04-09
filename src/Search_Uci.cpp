#include <iostream>
#include <sstream>
#include <string>
#include "Search.h"
#include "defines.h"

//-----------------------------------------------------
//! \brief Commande UCI : position
//!         Entrée d'une position
//!         et d'une série de coups
//-----------------------------------------------------
void Board::parse_position(std::istringstream &is)
{
    std::string token;

    token.clear();
    is >> token;

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

