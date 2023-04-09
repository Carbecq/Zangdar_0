#ifndef SEARCH_H
#define SEARCH_H


class Search;

#include <thread>
#include "defines.h"
#include "Board.h"
#include "Timer.h"
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
           OrderingInfo& m_info, bool m_log, int m_id);
    ~Search();

    // Point de départ de la recherche
    template <Color C> void iterDeep();

    // Fonction UCI : Search_Uci.cpp
    void stop();
    MOVE get_best() const { return best_move; }
    void test_value(const std::string &fen);

    U64  nodes;
    int  current_depth;

private:
    std::atomic<bool>   stopped;

    int     threadID;
    Board   my_board;
    Timer   my_timer;
    int     output;
    MOVE    best_move;
    int     best_score;
    bool    logUci;
    OrderingInfo my_orderingInfo;

    std::array<int, MAX_PLY> statEval;

    static constexpr int CONTEMPT        = 0;           // TODO : option ?
    static constexpr int RAZORING_MARGIN = 650;
    static constexpr int FUTILITYMARGINS[4] = {0, 200, 300, 500};
    static constexpr int NULL_MOVE_R        = 2;    // réduction de la profondeur de recherche

    template <Color C> int alpha_beta(Board &board, int ply, int alpha, int beta, int depth, bool do_NULL, MOVE* pv);
    template <Color C> int quiescence(Board &board, int ply, int alpha, int beta, MOVE* pv);

    void new_search();

    void show_uci_result(int depth, int best_score, U64 elapsed, MOVE *pv) const;
    void show_uci_best(const MOVE best_move) const;
    bool check_limits();
    void update_pv(MOVE *dst, MOVE *src, MOVE move) const;

    inline int futility_margin(int depth, bool improving) {
        return depth * 110 + ((improving) ? 75 : 0);
    }

};

#endif // SEARCH_H
