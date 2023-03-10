#include <iostream>
#include <thread>
#include <memory>

#include "defines.h"
#include "Search.h"

//========================================================
//! \brief  Affiche tous les coups possibles ainsi que leur valeur
//! \param  depth   profondeur max de recherche
//---------------------------------------------------------
void test_eval(const std::string& abc)
{
    std::string fen;
    if (abc.empty())
        fen = START_FEN;
    else
        fen = abc;

    std::string promobug = "8/p1R5/6p1/3k2Np/7P/5K2/1bp4r/8 b - - 14 60 ";

//    fen = "7k/3q4/8/1r1R2b1/8/3p4/8/7K w - - 0 1";
//    fen = "r3k3/1K6/8/8/8/8/8/8 w q - 0 1";
//    fen = "k7/8/8/1p6/5q2/1Q2P3/8/7K w - - 0 1";
//    fen = "7k/Rp1b3B/2BR3p/2Nr2Qn/q5qb/prqn1b2/1PP1P3/3R1K2 w - - 0 1"; // test mvvlva
fen = "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1 ";
fen = "6k1/5ppp/8/1pP5/PP6/4P3/8/6K1 w - - 3 1 ";
fen = "6k1/4Pppp/8/1pP5/PP6/4P3/4P3/6K1 w - - 3 1 ";

    Search* search = new Search(); //std::make_shared<Search>();
    search->test_value(fen);
    delete search;
}

//==========================================================
//! \brief  Effectue une recherche sur une position
//! \param[in]  str     détermine la position à tester
//! \param[in]  dmax    profondeur max
//! \param[in]  tmax    temps max en millisecondes

//----------------------------------------------------------
void test_think(const std::string& str, int dmax, int tmax)
{
    std::string bug     = "2r1r1k1/pp1q1ppp/3p1b2/3P4/3Q4/5N2/PP2RPPP/4R1K1 w - - bm Qg4";
//bug = "2q3k1/4rppp/4r3/8/8/8/3R1PPP/3R2K1 b - - 0 1";
//  bug = "3r2k1/3r1ppp/3r4/8/8/1B6/5PPP/R6K b - - 0 1";

   bug = "8/1p6/p5R1/k7/Prpp4/K7/1NP5/8 w - - am Rd6; bm Rb6 Rg5+; id ;";

    std::string fine_70 = "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - ";
    std::string quiesc  = "2r1r1k1/pp1q1ppp/3p1b2/3P4/3Q4/5N2/PP2RPPP/4R1K1 w - - bm Qg4";  // test quiescence

    std::string abc;
    Search* search = new Search(); //std::make_shared<Search>();


    if (str == "s")
        abc = SILVER2;
    else if (str == "f")
        abc = fine_70;
    else if (str == "k")
            abc = KIWIPETE;
    else if (str == "b")
            abc = bug;
    else if (str == "q")
            abc = quiesc;

    search->test_think(abc, dmax, tmax);

    delete search;
}
