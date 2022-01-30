#include "Board.h"


// http://www.rocechess.ch/perft.html

//========================================================
//! \brief Test perft
//! \param[in]  pos     position courante
//! \param[in]  dmax    profondeur de recherche
//! \return Nombre de coups trouvés
//--------------------------------------------------------
U64 Board::perft(int depth)
{
//    string camp[2] = { "Blanc", "Noir"};
//       int side = static_cast<int>(pos->side_to_move);
//       std::cout << "-------------------perft dmax = " << depth << " side = " << camp[side] << " ply = " << pos->ply << std::endl;

    Move* move;

    if (depth == 0)
        return (1);

//    gen_caps();
//    int nbr_caps = first_move[positions->ply + 1] - first_move[positions->ply];

    gen_moves();

    U64 total = 0;
//    U64 nbrc = 0;

//    for (int index = first_move[positions->ply]; index < first_move[positions->ply + 1]; index++)
//    {
//        move = &(moves[index]);

//        if (move->capture() || move->promotion()!= EMPTY)
//            nbrc++;
//    }

//    if (nbr_caps != nbrc)
//    {
//        printf("erreur caps : depth=%d : %d %d \n", depth, nbr_caps, nbrc);
//        exit(1);
//    }
//    else
//    {
////        printf(" caps ok : depth=%d : %d %d \n", depth, nbr_caps, nbrc);
//    }

    for (int index = first_move[positions->ply]; index < first_move[positions->ply + 1]; index++)
    {
        move = &(moves[index]);

        if (make_move(move) == true)
        {
            total += perft(depth-1);
    //      printf("   %s :  \n", moves[index].show(0).c_str());
            unmake_move(move);
        }
    }

    return(total);
}

//========================================================
//! \brief Test divide
//! \param[in]  dmax    profondeur de recherche
//! \return Nombre de coups trouvés
//--------------------------------------------------------
U64 Board::divide(int depth)
{
    std::string camp[2] = { "Blanc", "Noir"};
    int side = static_cast<int>(positions->side_to_move);

    std::cout << "-------------------perft dmax = " << depth << " side = " << camp[side] << " ply = " << positions->ply << std::endl;

    Move* move;

    gen_moves();

    U64 total[1000];
    U64 aaa = 0;


    for (int index = first_move[positions->ply]; index < first_move[positions->ply + 1]; index++)
    {
        move = &(moves[index]);

        if (make_move(move) == true)
        {
            total[index] = perft(depth-1);
            aaa += total[index];
            unmake_move(move);

            printf("%s : %d \n", moves[index].show(0).c_str(), total[index]);
        }
    }

    return(aaa);
}
