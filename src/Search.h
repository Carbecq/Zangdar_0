#ifndef SEARCH_H
#define SEARCH_H


class Search;

#include <thread>
#include "defines.h"
#include "Timer.h"

struct ThreadData {
    std::thread thread;
    Search*     search;
    MOVE        best_move;
    int         best_score;
    int         best_depth;
    U64         nodes;
}__attribute__((aligned(64)));

#include "Board.h"
#include "OrderingInfo.h"


// classe permettant de redéfinir mon 'locale'
// en effet, je n'en ai pas trouvé (windows ? mingw ?)
// de qui permet d'écrire un entier avec un séparateur : 1.000.000

class MyNumPunct : public std::numpunct<char>
{
protected:
    virtual char do_thousands_sep() const { return '.'; }
    virtual std::string do_grouping() const { return "\03"; }
};


class Search
{
public:
    Search();
    Search(const Board &m_board, const Timer& m_timer,
           OrderingInfo& m_info, bool m_log);
    ~Search();

    // Point de départ de la recherche
    template <Color C> void think(int id);

    // Fonction UCI : Search_Uci.cpp
    void stop();
    void test_value(const std::string &fen);

private:
    bool    stopped;
    Board   my_board;
    Timer   my_timer;
    bool    logUci;
    OrderingInfo my_orderingInfo;

    std::array<int, MAX_PLY> statEval;

    static constexpr int CONTEMPT    = 0;           // TODO : option ?
    static constexpr int NULL_MOVE_R = 2;    // réduction de la profondeur de recherche

    template <Color C> int alpha_beta(Board &board, int ply, int alpha, int beta, int depth, bool do_NULL, MOVE* pv, ThreadData* td);
    template <Color C> int quiescence(Board &board, int ply, int alpha, int beta, MOVE* pv, ThreadData* td);

    void show_uci_result(const ThreadData *td, U64 elapsed, MOVE *pv) const;
    void show_uci_best(const ThreadData *td) const;
    bool check_limits(const ThreadData *td) const;
    void update_pv(MOVE *dst, MOVE *src, MOVE move) const;

    static constexpr int  WINDOW = 23;     // aspiration windows

};

#endif // SEARCH_H
