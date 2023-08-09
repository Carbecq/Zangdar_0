#ifndef SEARCH_H
#define SEARCH_H


class Search;

#include <thread>
#include "defines.h"
#include "Timer.h"

struct PVariation {
    MOVE line[MAX_PLY+1];
    int  length;
};

struct ThreadData {
    std::thread thread;
    int         index;
    Search*     search;
    MOVE        best_move;
    int         best_score;
    int         best_depth;
    int         score;
    int         depth;
    int         seldepth;
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

private:
    bool    stopped;
    Board   board;
    Timer   timer;
    bool    logUci;
    OrderingInfo my_orderingInfo;

    std::array<int, MAX_PLY> eval_history;

    template <Color C> void iterative_deepening(ThreadData* td);
    template <Color C> int aspiration_window(int ply, PVariation& pv, ThreadData* td);
    template <Color C> int alpha_beta(int ply, int alpha, int beta, int depth, PVariation& pv, ThreadData* td);
    template <Color C> int quiescence(int ply, int alpha, int beta, ThreadData* td);

    void show_uci_result(const ThreadData *td, U64 elapsed, PVariation &pv) const;
    void show_uci_best(const ThreadData *td) const;
    void show_uci_current(MOVE move, int currmove, int depth) const;
    bool check_limits(const ThreadData *td) const;
    void update_pv(PVariation &pv, const PVariation &new_pv, const MOVE move) const;

    static constexpr int ASPIRATION_WINDOW = 23;     // aspiration windows
    static constexpr int CONTEMPT    = 0;           // TODO : option ?
    static constexpr int NULL_MOVE_R = 2;    // réduction de la profondeur de recherche
    static constexpr int CurrmoveTimerMS = 2500;

    int Reductions[2][32][32];

};

#endif // SEARCH_H
