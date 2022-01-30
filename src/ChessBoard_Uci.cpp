#include <iostream>
#include <sstream>
#include <string>

#include "defines.h"
#include "ChessBoard.h"


// We received a new game command.
void ChessBoard::new_game()
{
#ifdef DEBUG_CLASS
    std::cout << "ChessBoard::new_game" << std::endl;
#endif

    search.new_game();
}


void ChessBoard::position(std::istringstream &is)
{
#ifdef DEBUG_CLASS
    std::cout << "ChessBoard::position" << std::endl;
#endif

    search.position(is);
}

void ChessBoard::go(std::istringstream &is)
{
#ifdef DEBUG_CLASS
    std::cout << "ChessBoard::go" << std::endl;
#endif

    search.go(is);
}

void ChessBoard::stop()
{
#ifdef DEBUG_CLASS
    std::cout << "ChessBoard::stop" << std::endl;
#endif

        search.stop();
}

void ChessBoard::quit()
{
#ifdef DEBUG_CLASS
    std::cout << "ChessBoard::quit" << std::endl;
#endif

        search.stop();
}
