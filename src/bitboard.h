#ifndef LIBCHESS_BITBOARD_HPP
#define LIBCHESS_BITBOARD_HPP

#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <ostream>
#include "defines.h"
#include "bitmask.h"
#include <sstream>
#include <iostream>

//! \brief  crée un bitboard à partir d'une position
constexpr Bitboard square_to_bit(const int sq) noexcept { return(1ULL << sq);}

//! \brief  récupère le bit à la position donnée
[[nodiscard]] constexpr bool get_bit(Bitboard b, const int sq)  noexcept { return (b & (1ULL << sq)) ; }

//! \brief  met le bit à 1
constexpr void set_bit(Bitboard& b, const int sq) noexcept      { b |= (1ULL << sq);      }

//! \brief met le bit à 0
constexpr void unset_bit(Bitboard& b, const int sq) noexcept    { b &= ~(1ULL << sq);     }       // pop

//! \brief change le bit en son opposé
constexpr void flip(Bitboard& b, const int sq) noexcept     { b ^= (1ULL << sq);    }

//! \brief change le bit en son opposé
constexpr void flip2(Bitboard& b, const int sq1, const int sq2) noexcept { b ^= 1ULL << sq1 ^ 1ULL << sq2;    }

//! \brief met le LSB à zéro
constexpr void unset_lsb(Bitboard& b) noexcept { b &= b-1; }

//! \brief met le LSB à zéro, modifie le bitboard, retourne la position du LSB
[[nodiscard]] constexpr int next_square(Bitboard& b) noexcept {
    auto index = std::countr_zero(b);
    b &= b - 1;
    return index;
}

//! \brief retourne la position du LSB
//!     Get the first occupied square from a bitboard
//!     Finds the least significant 1-bit
//!     Fonction aussi nommée : bitScanForward
//!     Remplace __builtin_ctzll à partir de C++20
[[nodiscard]] constexpr int first_square(Bitboard b) noexcept {
    return std::countr_zero(b); // Returns the number of consecutive 0 bits in the value of x, starting from the least significant bit ("right").
}

//! \brief
//! bitScanReverse : Finds the most significant 1-bit
//! countl_zero(0) = 64
//! donc attention à : 63 - countl_zero(0) = -1 !!!
[[nodiscard]] constexpr int msb(Bitboard b)  noexcept {
    b |= 1;
    return 63 - std::countl_zero(b);    // Returns the number of consecutive 0 bits in the value of x, starting from the most significant bit ("left").
}

//! \brief version provenant de M42.h
//!
[[nodiscard]] constexpr  int msbM42(uint64_t b) noexcept
{
  b |= 1;

#if __cplusplus > 201703L
  return std::bit_width(b) - 1;
#elif defined(USE_INTRIN)
#if defined(_MSC_VER)
  unsigned long idx;
  _BitScanReverse64(&idx, b);
  return (int)idx;
#elif defined(__GNUC__)
  return 63 - __builtin_clzll(b);
#endif  // _MSC_VER
#endif  // __cplusplus

  const int BitScanTable[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
    57, 49, 41, 37, 28, 16,  3, 61,
    54, 58, 35, 52, 50, 42, 21, 44,
    38, 32, 29, 23, 17, 11,  4, 62,
    46, 55, 26, 59, 40, 36, 15, 53,
    34, 51, 20, 43, 31, 22, 10, 45,
    25, 39, 14, 33, 19, 30,  9, 24,
    13, 18,  8, 12,  7,  6,  5, 63
  };
  b |= (b |= (b |= (b |= (b |= b >> 1) >> 2) >> 4) >> 8) >> 16;
  return BitScanTable[((b | b >> 32) * 0x03f79d71b4cb0a89ULL) >> 58];
}


[[nodiscard]] constexpr int  Bcount(Bitboard b) noexcept {
    return std::popcount(b);
}

[[nodiscard]] constexpr bool Bempty(Bitboard b) noexcept { return b == 0ULL; }
[[nodiscard]] constexpr bool Bsubset(Bitboard a, Bitboard b) noexcept { return( (a & b) == a); }



/* Byte swap (= vertical mirror) */
constexpr Bitboard byteswap(Bitboard b)
{
#if defined(_MSC_VER)
    return _byteswap_uint64(b);
#elif defined(__GNUC__)
    return __builtin_bswap64(b);
#else

    b = ((b >> 8)  & 0x00FF00FF00FF00FFULL) | ((b & 0x00FF00FF00FF00FFULL) << 8);
    b = ((b >> 16) & 0x0000FFFF0000FFFFULL) | ((b & 0x0000FFFF0000FFFFULL) << 16);
    return (b >> 32) | (b << 32);
#endif
}


[[nodiscard]] constexpr Bitboard adjacent(Bitboard b)  noexcept {
return north(b) | south(b) | east(b) | west(b) | north_east(b) | north_west(b) | south_east(b) | south_west(b);
}

//======================================
//! \brief  Impression d'un Bitboard
//--------------------------------------
inline void PrintBB(const Bitboard bb)
{
    assert(bb);

    std::stringstream ss;
    int i = 56;

    ss << std::endl;
    ss << "  8 " ;

    while (i >= 0) {
        const auto sq = i;
        if (bb & square_to_bit(sq)) {
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

#endif
