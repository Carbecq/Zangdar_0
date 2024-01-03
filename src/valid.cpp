#include "bitboard.h"
#include "Board.h"
#include "Square.h"
#include <iostream>

template <Color C>
[[nodiscard]] constexpr bool Board::valid() const noexcept
{
//    std::cout << "valid debut" << std::endl;

#if defined USE_HASH
    U64 hash_1, hash_2;
    calculate_hash(hash_1, hash_2);
    if (hash != hash_1)
    {
        std::cout << "erreur hash 1" << std::endl;
        return false;
    }
    if (pawn_hash != hash_2)
    {
        std::cout << "erreur hash 2 : " << pawn_hash << "  " << hash_2 << std::endl;
        return false;
    }
#endif
    if (ep() != NO_SQUARE) {
        if (turn() == Color::WHITE && Square::rank(ep()) != 5) {
            std::cout << "erreur 1" << std::endl;
            return false;
        }
        if (turn() == Color::BLACK && Square::rank(ep()) != 2) {
            std::cout << "erreur 2" << std::endl;
            return false;
        }
    }

    if (Bcount(pieces_cp<Color::WHITE, PieceType::King>()) != 1) {
        std::cout << "erreur 3" << std::endl;
        return false;
    }

    if (Bcount(pieces_cp<Color::BLACK, PieceType::King>()) != 1) {
        std::cout << "erreur 4" << std::endl;
        return false;
    }

    if (colorPiecesBB[0] & colorPiecesBB[1]) {
        PrintBB(colorPiecesBB[0], "erreur 5");
        PrintBB(colorPiecesBB[1], "erreur 5");
        std::cout << "erreur 5" << std::endl;
        return false;
    }

    if (occupancy_p<PieceType::Pawn>() & (RANK_1_BB | RANK_8_BB)) {
        printf("%s \n", display().c_str());
        PrintBB(occupancy_p<PieceType::Pawn>(), "erreur 5");
        PrintBB(RANK_1_BB, "erreur 5");
        PrintBB(RANK_8_BB, "erreur 5");

        std::cout << "erreur 6" << std::endl;
        return false;
    }

    for (int i = Pawn; i <= King; ++i) {
        for (int j = i + 1; j <= King; ++j) {
            if (typePiecesBB[i] & typePiecesBB[j]) {
                std::cout << "erreur 7 " << std::endl;
                return false;
            }
        }
    }

    //    if ((colorPiecesBB[1] | colorPiecesBB[1]) != (typePiecesBB[0] | typePiecesBB[1] | typePiecesBB[2] | typePiecesBB[3] | typePiecesBB[4] | typePiecesBB[5])) {
    //        printf("%s \n\n", display().c_str());
    // PrintBB(colorPiecesBB[0]);
    // PrintBB(colorPiecesBB[1]);
    // PrintBB(typePiecesBB[0]);
    // PrintBB(typePiecesBB[1]);
    // PrintBB(typePiecesBB[2]);
    // PrintBB(typePiecesBB[3]);
    // PrintBB(typePiecesBB[4]);
    // PrintBB(typePiecesBB[5]);

    //        std::cout << "erreur 8" << std::endl;
    //        return false;
    //    }

    if (x_king[WHITE] != king_position<WHITE>())
    {
        std::cout << "erreur roi blanc" << std::endl;
        return(false);
    }

    if (x_king[BLACK] != king_position<BLACK>())
    {
        std::cout << "erreur roi noir" << std::endl;
        return(false);
    }

    for (int i=0; i<64; i++)
    {
        if (cpiece[i] != piece_on(i))
        {
            std::cout << "erreur piece case " << i << std::endl;
            return(false);
        }
    }

    //    std::cout << "11 " << std::endl;

    //    if (white_can_castle_k()) {
    //        if (!(Bitboard(king_position(Color::WHITE)) & bitboards::Rank1)) {
    //            return false;
    //        }
    //        if (piece_on(castle_rooks_from_[0]) != PieceType::Rook) {
    //            return false;
    //        }
    //    }
    //  std::cout << "12 " << std::endl;

    //    if (white_can_castle_q()) {
    //        if (!(Bitboard(king_position(Color::WHITE)) & bitboards::Rank1)) {
    //            return false;
    //        }
    //        if (piece_on(castle_rooks_from_[1]) != PieceType::Rook) {
    //            return false;
    //        }
    //    }
    //   std::cout << "13 " << std::endl;

    //    if (black_can_castle_k()) {
    //        if (!(Bitboard(king_position(Color::BLACK)) & bitboards::Rank8)) {
    //            return false;
    //        }
    //        if (piece_on(castle_rooks_from_[2]) != PieceType::Rook) {
    //            return false;
    //        }
    //    }
    //   std::cout << "14 " << std::endl;

    //    if (black_can_castle_q()) {
    //        if (!(Bitboard(king_position(Color::BLACK)) & bitboards::Rank8)) {
    //            return false;
    //        }
    //        if (piece_on(castle_rooks_from_[3]) != PieceType::Rook) {
    //            return false;
    //        }
    //    }

    //   std::cout << "valid fin " << std::endl;

    return true;
}

// Explicit instantiations.
template bool Board::valid<WHITE>() const noexcept;
template bool Board::valid<BLACK>() const noexcept;

