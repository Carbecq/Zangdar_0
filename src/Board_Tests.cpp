#include "Board.h"

//===========================================================
//! \brief  Test vérifiant la symétrie de l'évaluation
//-----------------------------------------------------------
bool Board::test_mirror(const std::string& line)
{
    int ev1 = 0; int ev2 = 0;
    bool r = true;
    //std::cout << "********************************************************" << std::endl;
    set_fen(line, true);

    //    std::cout << display() << std::endl;

    ev1 = evaluate();
    //    std::cout << "side = " << positions->side_to_move << " : ev1 = " << ev1 << std::endl;

    mirror_fen(line, true);
    //    std::cout << display() << std::endl;
    ev2 = evaluate();
    //    std::cout << "side = " << positions->side_to_move << " : ev2 = " << ev2 <<  std::endl;

    if(ev1 != ev2)
    {
        //        std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " << std::endl;//        display_ascii();
        r = false;
    }


    return(r);
}
