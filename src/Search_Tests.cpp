#include "defines.h"
#include "Search.h"


void Search::test_search(int depth, const std::string& line)
{
#ifdef DEBUG_CLASS
    std::cout << "Search::test_search" << std::endl;
#endif

    new_game();
    init_fen(line);

    setDepth(depth);
    setLogUci(true);
    setlogSearch(true);
    setlogTactics(false);
//    display_ascii();

    think();




    //    char WAC1[200] =  "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - -";
    //    init(WAC1);

    //    init(START_FEN);

    //    static const string FEN = "rnbqkbnr/pppppppp/8/8/8/8/P1PPPPPP/RNBQKBNR w KQkq - 0 1";
    //    init(FEN);

    //    char SILVER1[100] = "1rbq1rk1/1pp2pbp/p1np1np1/4p3/2PPP3/2N1BP2/PP1Q2PP/R1N1KB1R w KQ e6 ";
 //   char SILVER2[100] = "r1b1k2r/ppppnppp/2n2q2/2b5/3NP3/2P1B3/PP3PPP/RN1QKB1R w KQkq - 0 1";
 //   init(SILVER2);

    //    char BUG[200] =  "4k3/8/8/8/3n1p1p/1p1n1P1P/1P1P1PBP/2BR1KBB w - - 0 1";
    //    init(BUG);

    //   char WAC18[100] = "R7/P4k2/8/8/8/8/r7/6K1 w - -";
    //                                        b KQkq ep
    //   init(WAC18);

    std::string ZZ[5];
    ZZ[0] = "k7/8/8/8/q7/8/8/1R3R1K w - - 0 1";
    ZZ[1] = "5rk1/5Npp/8/3Q4/8/8/8/7K w - - 0 1";
    ZZ[2] = "2k5/8/8/8/p7/8/8/4K3 b - - 0 1";
    ZZ[3] = "8/8/8/8/8/8/R7/R3K2k w Q - 0 1";
    ZZ[4] = "7k/8/8/8/R2K3q/8/8/8 w - - 0 1";


    //   for (int i=0; i<5; i++)
    {
        //        init(ZZ[i]);


        //        display_ascii();

    }

}

bool Search::test_tactics(int depth, const std::string& line)
{
#ifdef DEBUG_CLASS
    std::cout << "Search::test_tactics" << std::endl;
#endif

    new_game();
    init_fen(line);
    setDepth(depth);
    setLogUci(false);
    setlogSearch(false);
    setlogTactics(true);
    think();

    bool found = false;
    for (auto & e : positions->best)
    {
        if(pv[0][0] == e) // attention au format de sortie de Move::show()
        {
            found = true;
            break;
        }
    }

    // NOTE : il est possible que dans certains cas, il faut donner
    // à la fois la case de départ et celle d'arrivée pour déterminer
    // réellement le coup.

    if (found)
    {
        std::cout << "ok" << std::endl;
        return(true);
    }
    else
    {
        //       display_ascii();

        std::cout << "-----------------meilleurs coups = ";
        for (auto & e : positions->best)
            std::cout << e << " ";
        std::cout << "; coup trouvé = " << pv[0][0] << std::endl;
        return(false);
    }

}

// https://www.talkchess.com/forum3/viewtopic.php?f=7&t=77777

void Search::test_gen(const std::string& line)
{
#ifdef DEBUG_CLASS
    std::cout << "Search::test_gen" << std::endl;
#endif

    new_game();
    setLogUci(false);
    setlogSearch(false);
    setlogTactics(true);

    init_fen(line);
    display_ascii();

    gen_moves();

    // Boucle sur tous les coups
    for (int index = first_move[positions->ply]; index < first_move[positions->ply + 1]; ++index)
    {
        if (make_move(&moves[index]) == true)
        {
            //        Square s = moves[i].from();
            //        PieceType  t = board[s]->type();
            printf("move %d : %s \n", index, moves[index].show().c_str());

            unmake_move(&moves[index]);
        }
        else
        {

        }
    }
}

//===========================================================
//! \brief  Test vérifiant la symétrie de l'évaluation
//-----------------------------------------------------------
bool Search::test_mirror(const std::string& line)
{
#ifdef DEBUG_CLASS
    std::cout << "Search::test_mirror" << std::endl;
#endif

    setLogUci(false);
    setlogSearch(false);
    setlogTactics(true);

    int ev1 = 0; int ev2 = 0;
    bool r = true;
//std::cout << "********************************************************" << std::endl;
    init_fen(line);
//    display_ascii();
    ev1 = evaluate(positions->side_to_move);
//    std::cout << "side = " << positions->side_to_move << " : ev1 = " << ev1 << std::endl;

    mirror_fen(line);
//    display_ascii();
    ev2 = evaluate(positions->side_to_move);
//    std::cout << "side = " << positions->side_to_move << " : ev2 = " << ev2 <<  std::endl;

    if(ev1 != ev2)
    {
//        std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " << std::endl;//        display_ascii();
        r = false;
    }


    return(r);
}

//===========================================================
//! \brief  Test de rapidité sur la position actuelle
//-----------------------------------------------------------
void Search::bench(int depth)
{
#ifdef DEBUG_CLASS
    std::cout << "Search::bench" << std::endl;
#endif

    setDepth(depth);
    setLogUci(true);
    setlogSearch(true);
    setlogTactics(false);
    display_ascii();

    auto start = std::chrono::high_resolution_clock::now();

    U64 total = Board::perft(getDepth());;

    auto end = std::chrono::high_resolution_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    printf("Moves   %10llu\n",  total);
    printf("Time    %9.3f s\n", ms/1000.0);
    printf("Moves/s %.3f\n",    (total/ms)/1000.0);

}
