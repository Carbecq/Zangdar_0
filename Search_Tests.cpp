#include "defines.h"
#include "Search.h"

//======================================================
//! \brief  Affiche tous les coups possibles ainsi que leur valeur
//!         L'affichage est fait dans l'ordre défini par la valeur du coup
//------------------------------------------------------
void Search::test_value(const std::string& abc )
{
    board = new Board();
    timer = new Timer();

    new_game();
    init_fen(abc);

    std::cout << board->display() << std::endl;

    MoveList ml;
    U32 move;

    printf("side = %s : evaluation = %d \n", side_name[board->side_to_move].c_str(), board->evaluate());


    // generate successor moves
//    board->legal_moves<WHITE>(ml);
//    order_moves(0, ml, 0);

    // look over all moves
//    for (int index=0; index<ml.count; index++)
//    {
//        // prend le meilleur coup, basé sur la valeur definie dans la génération des coups
//        //  > MvvLva
//        PickNextMove(ml, index);

//        move = ml.moves[index];

//        // execute current move
//        board->make_move<WHITE>(move);

//        printf("side = %s : %s : value=%d score=%d \n", SideName[board->side_to_move].c_str(), Move::name(move).c_str(), ml.values[index], board->evaluate());

//        // retract current move
//        board->undo_move<WHITE>();
//    }
}

void Search::test_think(const std::string& line, int dmax, int tmax)
{
    board = new Board();
    timer = new Timer();

    new_game();
    init_fen(line);

    std::cout << board->display() << std::endl;

    if (dmax != 0)
        setDepth(dmax);
    if (tmax != 0)
        setTime(tmax);

    setLogUci(true);
    setlogSearch(true);
    setlogTactics(false);

    if (board->side_to_move == WHITE)
        think<WHITE>(board, timer);
    else
        think<BLACK>(board, timer);
}

//=============================================================
//! \brief Réalisaion d'un test tactique
//! et comparaison avec le résultat
//-------------------------------------------------------------
bool Search::test_tactics(const std::string& line, int dmax, int tmax, U64& total_nodes, U64& total_time)
{
    board = new Board();
    timer = new Timer();

    new_game();

    setLogUci(false);
    setlogSearch(false);
    setlogTactics(true);

    init_fen(line);

    setOutput(1);

    if (dmax != 0)
        setDepth(dmax);
    if (tmax != 0)
        setTime(tmax);

    auto start = std::chrono::high_resolution_clock::now();

    if (board->side_to_move == WHITE)
        think<WHITE>(board, timer);
    else
        think<BLACK>(board, timer);

    auto end = std::chrono::high_resolution_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    total_time  += ms;
    total_nodes += nodes;

    std::string str1 = Move::show(best, 1);
    std::string str2 = Move::show(best, 2);
    std::string str3 = Move::show(best, 3);

    bool found = false;
    std::string abc;

    // NOTE : il est possible que dans certains cas, il faut donner
    // à la fois la case de départ et celle d'arrivée pour déterminer
    // réellement le coup.
    for (auto & e : board->best_moves)
    {
        // On vire tous les caractères inutiles à la comparaison
        for (char c : std::string("+#!?"))
        {
            e.erase(std::remove(e.begin(), e.end(), c), e.end());
        }

        if(str1==e || str2 == e || str3 == e) // attention au format de sortie de Move::show()
        {
            found = true;
            abc   = e;
            break;
        }
    }


    if (found)
    {
        std::cout << "ok : " << abc << std::endl;
        return(true);
    }
    else
    {
        //       display_ascii();

        std::cout << "-----------------meilleurs coups = ";
        for (auto & e : board->best_moves)
            std::cout << e << " ";
        std::cout << "; coup trouvé = " << str1 << std::endl;
        return(false);
    }

}

//===========================================================
//! \brief  Test vérifiant la symétrie de l'évaluation
//-----------------------------------------------------------
bool Search::test_mirror(const std::string& line)
{
    board = new Board();
    timer = new Timer();

    setLogUci(false);
    setlogSearch(false);
    setlogTactics(true);

    int ev1 = 0; int ev2 = 0;
    bool r = true;
    //std::cout << "********************************************************" << std::endl;
    init_fen(line);
    //    std::cout << display() << std::endl;

    ev1 = board->evaluate();
    //    std::cout << "side = " << positions->side_to_move << " : ev1 = " << ev1 << std::endl;

    board->mirror_fen(line, logTactics);
    //    std::cout << display() << std::endl;
    ev2 = board->evaluate();
    //    std::cout << "side = " << positions->side_to_move << " : ev2 = " << ev2 <<  std::endl;

    if(ev1 != ev2)
    {
        //        std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " << std::endl;//        display_ascii();
        r = false;
    }


    return(r);
}
