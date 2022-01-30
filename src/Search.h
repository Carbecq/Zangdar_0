#ifndef SEARCH_H
#define SEARCH_H

class Search;

#include <array>
#include "defines.h"
#include "Move.h"
#include "Position.h"
#include "Timer.h"
#include "Zobrist.h"
#include "Board.h"
#include "HashTable.h"

// classe permettant de redéfinir mon 'locale'
// en effet, je n'en ai pas trouvé (windows ? mingw ?)
// de qui permet d'écrire un entier avec un séparateur : 1.000.000

class MyNumPunct : public std::numpunct<char>
{
protected:
    virtual char do_thousands_sep() const { return '.'; }
    virtual std::string do_grouping() const { return "\03"; }
};

class Search : public Board
{
public:
    Search();
    ~Search();

    bool stopped;
    U64  nodes;             // nodes searched
    int  limitCheckCount;
    int  seldepth;          // Maximum selective depth reached

    void init();
    void think();
    void init_fen(const std::string& fen);
    void reset();

    // Search_Tests.cpp
    void test_search(int depth, const std::string& line);
    bool test_tactics(int depth, const std::string& line);
    void test_gen(const std::string& line);
    bool test_mirror(const std::string& line);
    void bench(int depth);

    // Fonction UCI : Search_Uci.cpp
    void position(std::istringstream &is);
    void stop();
    void go(std::istringstream &is);
    void new_game();

    void setDepth(int depth);
    void setLogUci(bool b)      { logUci = b;       }
    void setlogSearch(bool b)   { logSearch = b;    }
    void setlogTactics(bool b)  { logTactics = b;   }
    void setTime(int time);
    void setInfinite(bool infini);

    int  getDepth() const { return(timer.getSearchDepth());}

private:
    bool        logUci;
    bool        logSearch;
    bool        logTactics;
    std::string _bestMove;      //  Best move found on last search.
    int         _bestScore;     //  Score corresponding to _bestMove

    Timer       timer;
    HashTable   hashtable;

    // code venant de TSCP
    /* a "triangular" PV array; for a good explanation of why a triangular
       array is needed, see "How Computers Play Chess" by Levy and Newborn. */

    std::string pv[MAX_PLY][MAX_PLY];
    int  pv_length[MAX_PLY];

    I32  alpha_beta(int alpha, int beta, int depth, bool doNull);
    I32  quiescence(int alpha, int beta);
    int  IsRepetition();
    void PickNextMove(int index);
    void new_search();
    int  search_root(int alpha, int beta, int depth);

    void show_uci_result(int currentDepth, int seldepth, int bestScore, int elapsed) const;
    void show_uci_best(const std::string& name) const;
    bool checkLimits();

    int MoveExists(Position *pos, int move);

};

#endif // SEARCH_H
