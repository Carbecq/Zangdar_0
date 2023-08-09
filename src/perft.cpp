#include <iostream>
#include "Board.h"
#include "defines.h"
#include <cassert>
#include "Move.h"

template <Color C> [[nodiscard]] std::uint64_t Board::perft(const int depth) noexcept
{
    if (depth == 0)
        return 1;

    MoveList ml;
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

    MoveList ml;
    legal_moves<C>(ml);

    for (size_t index = 0; index < ml.count; index++)
    {
        move = ml.moves[index];

//        const auto to       = Move::dest(move);
//        const auto from     = Move::from(move);
//        const auto piece    = Move::piece(move);
//        const auto captured = Move::captured(move);
//        const auto promo    = Move::promotion(move);

//        printf("move  = (%s) \n", Move::name(move).c_str());
//        binary_print(move);
//        printf("from  = %s \n", square_name[from].c_str());
//        printf("dest  = %s \n", square_name[to].c_str());
//        printf("piece = %s \n", piece_name[piece].c_str());
//        printf("capt  = %s \n", piece_name[captured].c_str());
//        printf("promo = %s \n", piece_name[promo].c_str());
//        printf("flags = %d \n", Move::flags(move));

        make_move<C>(move);
        total[index] = perft<~C>(depth - 1);
        nodes += total[index];
        undo_move<C>();
        std::cout << Move::name(move) << " : " << total[index] << std::endl;
    }

    return nodes;
}

// Explicit instantiations.
template std::uint64_t Board::perft<WHITE>(const int depth) noexcept;
template std::uint64_t Board::perft<BLACK>(const int depth) noexcept;
template std::uint64_t Board::divide<WHITE>(const int depth) noexcept;
template std::uint64_t Board::divide<BLACK>(const int depth) noexcept;

