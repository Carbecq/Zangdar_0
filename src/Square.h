#ifndef SQUARE_H
#define SQUARE_H

#include <string>
#include "color.h"
#include "bitmask.h"
#include "bitboard.h"

enum Squares : int {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NO_SQUARE
};

const std::string square_name[] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};

enum rank : int { RANK_1 = 0, RANK_2 = 1, RANK_3 = 2, RANK_4 = 3, RANK_5 = 4, RANK_6 = 5, RANK_7 = 6, RANK_8 = 7, NO_RANK = 8 };
enum file : int { FILE_A = 0, FILE_B = 1, FILE_C = 2, FILE_D = 3, FILE_E = 4, FILE_F = 5, FILE_G = 6, FILE_H = 7, NO_FILE = 8 };

namespace Square {

// The constexpr specifier declares that it is possible to evaluate the value of the function or variable at compile time.

[[nodiscard]] constexpr int square(const int f, const int r) noexcept {
    return (8*r + f);
}

[[nodiscard]] inline int square(const std::string& str) noexcept {
    const int file = str[0] - 'a';
    const int rank = str[1] - '1';
    return(rank * 8 + file);
}

inline std::ostream &operator<<(std::ostream &os, const int square) noexcept {
    os << square_name[square];
    return os;
}

[[nodiscard]] constexpr int rank(int square) noexcept {
    return (square / 8);
}

[[nodiscard]] constexpr int file(int square) noexcept {
    return (square % 8);
}

[[nodiscard]] constexpr int north(int square) noexcept {
    return (square + 8);
}

[[nodiscard]] constexpr int north_west(int square) noexcept {
    return (square + 7);
}

[[nodiscard]] constexpr int west(int square) noexcept {
    return (square - 1);
}

[[nodiscard]] constexpr int south_west(int square)  noexcept
{
    return (square - 9);
}

[[nodiscard]] constexpr int south(int square)  noexcept
{
    return (square - 8);
}

[[nodiscard]] constexpr int south_east(int square) noexcept {
    return (square - 7);
}

[[nodiscard]] constexpr int east(int square) noexcept {
    return (square + 1);
}

[[nodiscard]] constexpr int north_east(int square) noexcept {
    return (square + 9);
}

[[nodiscard]] constexpr int south_south(int square)  noexcept
{
    return (square - 16);
}

[[nodiscard]] constexpr int north_north(int square) noexcept {
    return (square + 16);
}

// flip vertically
[[nodiscard]] constexpr int flip(int sq) noexcept {
    return sq ^ 56;
}

[[nodiscard]] constexpr int flip(Color C, int sq) noexcept {
    return ( (C == WHITE) ? sq : sq ^ 56);
}

//! \brief Contrôle si la case "sq" est sur la rangée précédant la promotion (7ème/2ème)
template <Color C>
[[nodiscard]] constexpr bool is_on_seventh_rank(const int sq) {
    return (Promoting_Rank[C] & square_to_bit(sq));
}

//! \brief Contrôle si la case "sq" est sur la rangée de promotopn (8ème, 1ère)
template <Color C>
[[nodiscard]] constexpr bool is_promotion(const int sq) {
    return (Promotion_Rank[C] & square_to_bit(sq));
}

//! \brief Contrôle si la case "sq" est sur la rangée de départ (2ème/7ème)
template <Color C>
[[nodiscard]] constexpr bool is_on_second_rank(const int sq) {
    return(Starting_Rank[C] & square_to_bit(sq));
}






} // namespace





#endif // SQUARE_H
