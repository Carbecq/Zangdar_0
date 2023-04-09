#include "defines.h"
#include "Search.h"

//======================================================
//! \brief  Affiche tous les coups possibles ainsi que leur valeur
//!         L'affichage est fait dans l'ordre défini par la valeur du coup
//------------------------------------------------------
void Search::test_value(const std::string& fen )
{
    Board b;
    stop();
    new_search();
 //   Transtable.clear();
    b.set_fen(fen, false);

    std::cout << b.display() << std::endl;

    MoveList ml;
    U32 move;

    printf("side = %s : evaluation = %d \n", side_name[b.side_to_move].c_str(), b.evaluate());


    // generate successor moves
    b.legal_moves<WHITE>(ml);

    // look over all moves
    for (int index=0; index<ml.count; index++)
    {
        move = ml.moves[index];

        // execute current move
        b.make_move<WHITE>(move);

       bool doCheck    = b.is_in_check<BLACK>();

        printf("side = %s : %s : value=%d score=%d \n", side_name[b.side_to_move].c_str(),
                Move::name(move).c_str(), ml.values[index], b.evaluate());
       if (doCheck)
           printf("blanc fait echec \n");
       else
           printf("blanc ne fait pas echec \n");

        // retract current move
        b.undo_move<WHITE>();
    }
}


