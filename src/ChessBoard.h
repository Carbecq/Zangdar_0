#ifndef CHESSBOARD_H
#define CHESSBOARD_H

class ChessBoard;

#include <string>
#include <bitset>
#include <array>

#include "defines.h"
#include "Timer.h"
#include "Search.h"

//  Classe servant à lancer une partie.

class ChessBoard
{
public:
    ChessBoard();
    ~ChessBoard();

    Search  search;

    void init();
    void init(const std::string& fen);
    void reset();

    // Fonction UCI
    void position(std::istringstream &is);
    void new_game();
    void go(std::istringstream &is);
    void stop();
    void quit();

    // Tests.cpp
    U64  test_perft(int depth_max);
    U64  test_divide(int depth_max);
    void test_gen(const std::string& line);
    void test_search(int depth, const std::string& line);
    bool test_tactics(int depth, const std::string& line);
    bool test_mirror(const std::string& line);

    void setOutput(int i) { search.setOutput(i); }

private:

};

#endif // CHESSBOARD_H
