#include <iostream>
#include <sstream>
#include <string>
#include <cstring>

#include "Timer.h"
#include "defines.h"
#include "ChessBoard.h"
#include "Uci.h"


Uci::Uci()
{

}
void Uci::run()
{
    std::string token;
    std::string line;

    ChessBoard* CB = new ChessBoard();

    // print engine info
    std::cout << "id name Zangdar " << VERSION << std::endl;
    std::cout << "id author Philippe Chevalier" << std::endl;
    std::cout << "uciok" << std::endl;

    // main loop
    while(getline(std::cin, line))
    {
        std::istringstream is(line);

        token.clear();
        is >> std::skipws >> token;

        //-------------------------------------------- gui -> engine

        if (token == "uci")
        {
            CB->setOutput(4);

            // print engine info
            std::cout << "id name Zangdar " << VERSION << std::endl;
            std::cout << "id author Philippe Chevalier" << std::endl;
            std::cout << "uciok" << std::endl;
        }

        else if (token == "isready")
        {
            // synchronize the engine with the GUI
            std::cout << "readyok" << std::endl;
            continue;
        }

        else if (token == "position")
        {
            // set up the position described in fenstring
            CB->position(is);
        }

        else if (token == "ucinewgame")
        {
            // the next search (started with "position" and "go") will be from
            // a different game.
            CB->new_game();
        }

        else if (token == "go")
        {
            // start calculating on the current position
            CB->go(is);
        }

        else if (token == "quit")
        {
            CB->quit();
            // quit the program as soon as possible
            exit(0);
        }

        else if (token == "stop")
       {
            CB->stop();
        }
            // stop calculating as soon as possible

        //        else if (!strncmp(input, "setoption name Hash value ", 26)) {
        //            // init MB
        //            sscanf(input,"%*s %*s %*s %*s %d", &mb);

        //            // adjust MB if going beyond the aloowed bounds
        //            if(mb < 4) mb = 4;
        //            if(mb > max_hash) mb = max_hash;

        //            // set hash table size in MB
        //            std::cout << "    Set hash table size to %dMB\n", mb);
        //            init_hash_table(mb);
        //        }

        //------------------------------------------------------- engine -> gui
    }

    delete CB;
}

