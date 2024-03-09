#ifndef LIBCHESS_BITBOARD_HPP
#define LIBCHESS_BITBOARD_HPP

#include <bit>
#include <cassert>
#include <ostream>
#include "defines.h"
#include <sstream>
#include <iostream>
#include "types.h"
#include "bitmask.h"

namespace BB {

//! \brief  crée un bitboard à partir d'une case
constexpr Bitboard sq2BB(const int sq) noexcept { return(1ULL << sq);}

//! \brief  récupère le bit à la position donnée
[[nodiscard]] constexpr bool get_bit(Bitboard b, const int sq)  noexcept { return (b & (1ULL << sq)) ; }

//! \brief  met le bit à 1
constexpr void set_bit(Bitboard& b, const int sq) noexcept      { b |= (1ULL << sq);      }

//! \brief met le bit à 0
constexpr void clear_bit(Bitboard& b, const int sq) noexcept    { b &= ~(1ULL << sq);     }

//! \brief teste si le bit est mis
constexpr bool test_bit(const Bitboard b, const int sq) noexcept    { return ( b & (1ULL << sq));     }

//! \brief change le bit en son opposé
constexpr void toggle_bit(Bitboard& b, const int sq) noexcept     { b ^= (1ULL << sq);    }

//! \brief change les deux bits en leur opposé
constexpr void toggle_bit2(Bitboard& b, const int sq1, const int sq2) noexcept { b ^= (1ULL << sq1) ^ (1ULL << sq2);    }

//! \brief met le LSB à zéro
constexpr void unset_lsb(Bitboard& b) noexcept { b &= b-1; }

//! \brief met le LSB à zéro, modifie le bitboard, retourne la position du LSB
//! aussi appelée poplsb
[[nodiscard]] constexpr int pop_lsb(Bitboard& b) noexcept {
    int index = std::countr_zero(b);       // position du LSB
    b &= b - 1;                             // met le LSB à 0
    return index;
}

[[nodiscard]] constexpr int PYRRHIC_pop_lsb(Bitboard* b) noexcept {
    auto index = std::countr_zero(*b);
    *b &= *b - 1;
    return index;
}

//! \brief retourne la position du LSB
//!     Get the first occupied square from a bitboard
//!     Finds the least significant 1-bit
//!     Returns the number of consecutive 0 bits in the value of x, starting from the least significant bit ("right").
//!     Fonction aussi nommée : bitScanForward, getlsb
//!     Remplace __builtin_ctzll à partir de C++20
[[nodiscard]] constexpr int get_lsb(Bitboard b) noexcept {
    return std::countr_zero(b);
}

//! \brief
//! bitScanReverse : Finds the most significant 1-bit
//! Returns the number of consecutive 0 bits in the value of x, starting from the most significant bit ("left").
//! countl_zero(0) = 64
//! donc attention à : 63 - countl_zero(0) = -1 !!!
[[nodiscard]] constexpr int get_msb(Bitboard b)  noexcept {
    b |= 1;
    return 63 - std::countl_zero(b);
}

//! \brief  Donne le nombre de bits à 1 du bitboard
[[nodiscard]] constexpr int  count_bit(Bitboard b) noexcept { return std::popcount(b);}


//! \brief  Teste si le bitboard est vide
[[nodiscard]] constexpr bool empty(Bitboard b) noexcept { return b == 0ULL; }


//! \brief Teste si le bitboard a plus d'un set à 1
[[nodiscard]] constexpr bool multiple(Bitboard b) {
    return (b & (b - 1));
}

//! \brief Teste si le bitboard n'a qu'un seul bit à 1
[[nodiscard]] constexpr bool single(Bitboard b) {
    return (b && !multiple(b));
}




/* Byte swap (= vertical mirror) */
constexpr Bitboard byte_swap(Bitboard b)
{
#if defined(__GNUC__)
    return __builtin_bswap64(b);
#elif defined(__llvm__)
    return __builtin_bswap64(b);
#elif defined(_MSC_VER)
    return _byteswap_uint64(b);
#else
    b = ((b >> 8)  & 0x00FF00FF00FF00FFULL) | ((b & 0x00FF00FF00FF00FFULL) << 8);
    b = ((b >> 16) & 0x0000FFFF0000FFFFULL) | ((b & 0x0000FFFF0000FFFFULL) << 16);
    return (b >> 32) | (b << 32);
#endif
}

[[nodiscard]] constexpr Bitboard squaresOfMatchingColour(int sq) {
    assert(0 <= sq && sq < N_SQUARES);
    return BB::test_bit(LightSquares, sq) ? LightSquares : DarkSquares;
}



[[nodiscard]] constexpr Bitboard north(Bitboard b)      noexcept { return (b << 8); }
[[nodiscard]] constexpr Bitboard south(Bitboard b)      noexcept { return (b >> 8); }
[[nodiscard]] constexpr Bitboard east(Bitboard b)       noexcept { return ((b << 1) & NOT_FILE_A_BB); }
[[nodiscard]] constexpr Bitboard west(Bitboard b)       noexcept { return ((b >> 1) & NOT_FILE_H_BB); }
[[nodiscard]] constexpr Bitboard north_east(Bitboard b) noexcept { return((b << 9) & NOT_FILE_A_BB); }
[[nodiscard]] constexpr Bitboard south_east(Bitboard b) noexcept {  return((b >> 7) & NOT_FILE_A_BB); }
[[nodiscard]] constexpr Bitboard south_west(Bitboard b) noexcept { return((b >> 9) & NOT_FILE_H_BB); }
[[nodiscard]] constexpr Bitboard north_west(Bitboard b) noexcept { return((b << 7) & NOT_FILE_H_BB); }

[[nodiscard]] constexpr Bitboard north_north(Bitboard b)  noexcept { return (b << 16); }
[[nodiscard]] constexpr Bitboard south_south(Bitboard b)  noexcept { return (b >> 16); }

[[nodiscard]] constexpr Bitboard north_north_east(Bitboard b) noexcept { return((b << 17) & NOT_FILE_A_BB); }
[[nodiscard]] constexpr Bitboard north_north_west(Bitboard b) noexcept { return((b << 15) & NOT_FILE_H_BB); }

[[nodiscard]] constexpr Bitboard south_south_east(Bitboard b) noexcept { return (b >> 15 & NOT_FILE_A_BB); }
[[nodiscard]] constexpr Bitboard south_south_west(Bitboard b) noexcept { return (b >> 17 & NOT_FILE_H_BB); }

[[nodiscard]] constexpr Bitboard east_east_north(Bitboard b)  noexcept { return (b << 10 & NOT_FILE_AB_BB); }
[[nodiscard]] constexpr Bitboard east_east_south(Bitboard b)  noexcept { return (b >> 6  & NOT_FILE_AB_BB); }

[[nodiscard]] constexpr Bitboard west_west_north(Bitboard b)  noexcept { return (b << 6   & NOT_FILE_HG_BB); }
[[nodiscard]] constexpr Bitboard west_west_south(Bitboard b)  noexcept { return (b >> 10  & NOT_FILE_HG_BB); }

//! \brief Décale d'une case dans la direction donnée
template <Direction D>
[[nodiscard]] constexpr Bitboard shift(const Bitboard b)
{
    switch (D)
    {
    case NORTH:
        return b << 8;
        break;
    case NORTH_EAST:
        return (b << 9) & NOT_FILE_A_BB;
        break;
    case EAST:
        return (b << 1) & NOT_FILE_A_BB;
        break;
    case SOUTH_EAST:
        return (b >> 7) & NOT_FILE_A_BB;
        break;
    case SOUTH:
        return b >> 8;
        break;
    case SOUTH_WEST:
        return (b >> 9) & NOT_FILE_H_BB;
        break;
    case WEST:
        return (b >> 1) & NOT_FILE_H_BB;
        break;
    case NORTH_WEST:
        return (b << 7) & NOT_FILE_H_BB;
        break;
    default:
        return b;
    }
}

// https://www.chessprogramming.org/Pawn_Fills
//! \brief fill the bitboard to the north
[[nodiscard]] inline Bitboard north_fill(Bitboard b)
{
    b |= (b <<  8);
    b |= (b << 16);
    b |= (b << 32);
    return b;
}

//! \brief fill the bitboard to the south
[[nodiscard]] inline Bitboard south_fill(Bitboard b)
{
    b |= (b >>  8);
    b |= (b >> 16);
    b |= (b >> 32);
    return b;
}

template <Direction D>
[[nodiscard]] inline Bitboard fill(Bitboard b)
{
    if constexpr (D == NORTH)
    {
        b |= (b <<  8);
        b |= (b << 16);
        b |= (b << 32);
    }
    else
    {
        b |= (b >>  8);
        b |= (b >> 16);
        b |= (b >> 32);
    }
    return b;
}




// Returns the combined attack bitboard of all pawns in the given bitboard
template <Color C>
[[nodiscard]] constexpr Bitboard pawn_attacks_span(Bitboard pawns) {
    if constexpr (C == WHITE)
        return BB::north_west(pawns) | BB::north_east(pawns);
    else
        return south_west(pawns) | south_east(pawns);
}
template <Color C>
[[nodiscard]] constexpr Bitboard pawn_attacks_span(Bitboard pawns, Bitboard targets) {
    if constexpr (C == WHITE)
        return targets & (BB::north_west(pawns) | BB::north_east(pawns));
    else
        return targets & (south_west(pawns) | south_east(pawns));
}

// Returns the combined attack bitboard of all pawns in the given bitboard
template <Color C>
[[nodiscard]] constexpr Bitboard pawn_attacks_double(Bitboard pawns) {
    if constexpr (C == WHITE)
        return BB::north_west(pawns) & BB::north_east(pawns);
    else
        return south_west(pawns) & south_east(pawns);
}

template <Color C>
[[nodiscard]] constexpr Bitboard pawn_advance(Bitboard pawns, Bitboard occupied) {
    if constexpr (C == WHITE)
        return ~occupied & BB::north(pawns);
    else
        return ~occupied & south(pawns);
}
//======================================
//! \brief  Impression d'un Bitboard
//--------------------------------------
inline void PrintBB(const Bitboard bb, const std::string& message)
{
    std::cout << "--------------------------- " << message << std::endl;
    std::stringstream ss;
    int i = 56;

    ss << std::endl;
    ss << "  8 " ;

    while (i >= 0) {
        const auto sq = i;
        if (bb & BB::sq2BB(sq)) {
            ss << "1 ";
        } else {
            ss << ". ";
        }

        if (i % 8 == 7)
        {
            if (i/8 != 0)
                ss << "\n  " << i/8 << ' ';
            i -= 16;
        }

        i++;
    }
    ss << "\n    a b c d e f g h\n\n";

    std::cout << ss.str() << std::endl;
}

}; // namespace

#endif
