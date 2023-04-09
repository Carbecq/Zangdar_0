#include <iostream>
#include "Board.h"
#include "defines.h"
#include <cassert>

template <Color C> [[nodiscard]] std::uint64_t Board::perft(const int depth) noexcept
{
    if (depth == 0)
        return 1;

    MoveList ml; // = the_move_list[ply];
    legal_moves<C>(ml);

    // on génère des coups légaux, on peut
    // faire un bulk-counting

    //TODO enlever commentaire
    if (depth == 1)
        return ml.count;

    //    std::string decal = "";
    //    for (int k=0; k<depth; k++)
    //        decal += "  ";

    U64 total = 0;
    U32 move;

    // ml.moves est un "array", il faut utiliser ml.count
    for (size_t index = 0; index < ml.count; index++)
    {
        move = (ml.moves[index]);
        //        std::cout << "   " <<decal <<  Move::name(move) << " : " << std::endl;

        make_move<C>(move);
        total += perft<~C>(depth-1);
        undo_move<C>();
    }

    return total;
}

template <Color C> [[nodiscard]] std::uint64_t Board::divide(const int depth) noexcept
{
    U64 nodes  = 0ULL;
    U64 total[256];
    U32 move;

    MoveList ml; // = the_move_list[ply];
    legal_moves<C>(ml);

    for (int index = 0; index < ml.count; index++)
    {
        move = ml.moves[index];

        make_move<C>(move);
        total[index] = perft<~C>(depth - 1);
        nodes += total[index];
         undo_move<C>();
        printf(" %s : % llu \n", Move::name(move).c_str(),  total[index]);
    }

    return nodes;
}

// Explicit instantiations.
template std::uint64_t Board::perft<WHITE>(const int depth) noexcept;
template std::uint64_t Board::perft<BLACK>(const int depth) noexcept;
template std::uint64_t Board::divide<WHITE>(const int depth) noexcept;
template std::uint64_t Board::divide<BLACK>(const int depth) noexcept;

