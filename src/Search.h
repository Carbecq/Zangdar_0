#ifndef SEARCH_H
#define SEARCH_H

static constexpr int NULL_MOVE_R        = 2;    // réduction de la profondeur de recherche

class Search;

#include <thread>
#include "defines.h"
#include "Board.h"
#include "Timer.h"
#include "TranspositionTable.h"

// classe permettant de redéfinir mon 'locale'
// en effet, je n'en ai pas trouvé (windows ? mingw ?)
// de qui permet d'écrire un entier avec un séparateur : 1.000.000

class MyNumPunct : public std::numpunct<char>
{
protected:
    virtual char do_thousands_sep() const { return '.'; }
    virtual std::string do_grouping() const { return "\03"; }
};

class Search // : public Board
{
public:
    Search();
    ~Search();

    std::atomic<bool> stopped;
    U64  nodes;             // nodes searched

    void goThink();

    template <Color C> void think(Board* m_board, Timer* m_timer);

    void init_fen(const std::string& fen);
    void reset();

    // Fonction UCI : Search_Uci.cpp
    void position(std::istringstream &is);
    void stop();
    void new_game();
    void quit();

    void setDepth(int depth);
    void setLogUci(bool b)      { logUci = b;       }
    void setlogSearch(bool b)   { logSearch = b;    }
    void setlogTactics(bool b)  { logTactics = b;   }
    void setOutput(int o)       { output = o;       }
    void setTime(int time);
    void setInfinite(bool infini);

    void test_value(const std::string &abc);
    void test_think(const std::string& line, int dmax, int tmax);
    bool test_tactics(const std::string& line, int dmax, int tmax, U64& total_nodes, U64& total_time);
    bool test_mirror(const std::string& line);


private:
    bool        logUci;
    bool        logSearch;
    bool        logTactics;
    Board*      board;
    Timer*      timer;
    TranspositionTable   transtable;
    int         output;
    MOVE        best;

    int     searchHistory[2][6][64];
    U32     searchKillers[2][MAX_PLY];      // Killer Moves, 2 pour chaque profondeur de recherche

    std::array<int, MAX_PLY> statEval;

    template <Color C> int alpha_beta(int ply, int alpha, int beta, int depth, bool do_NULL, MOVE* pv );
    template <Color C> int quiescence(int ply, int alpha, int beta, MOVE* pv);

    void PickNextMove(MoveList &move_list, int index);
    void new_search();
    void setKillers(U32 move, int ply);
    void order_moves(int ply, MoveList& move_list, U32 PvMove);

    void show_uci_result(int currentDepth, int best_score, U64 elapsed, MOVE *pv) const;
    void show_uci_best(const MOVE best_move) const;
    bool check_limits();
    void update_pv(MOVE *dst, MOVE *src, MOVE move) const;






};


#endif // SEARCH_H
