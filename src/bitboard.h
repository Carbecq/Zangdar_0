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
#include "types.h"

//! \brief  crée un bitboard à partir d'une case
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
constexpr void flip2(Bitboard& b, const int sq1, const int sq2) noexcept { b ^= (1ULL << sq1) ^ (1ULL << sq2);    }

//! \brief met le LSB à zéro
constexpr void unset_lsb(Bitboard& b) noexcept { b &= b-1; }

//! \brief met le LSB à zéro, modifie le bitboard, retourne la position du LSB
//! aussi appelée poplsb
[[nodiscard]] constexpr int next_square(Bitboard& b) noexcept {
    auto index = std::countr_zero(b);
    b &= b - 1;
    return index;
}

[[nodiscard]] constexpr int PYRRHIC_next_square(Bitboard* b) noexcept {
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
[[nodiscard]] constexpr int first_square(Bitboard b) noexcept {
    return std::countr_zero(b);
}

//! \brief
//! bitScanReverse : Finds the most significant 1-bit
//! countl_zero(0) = 64
//! donc attention à : 63 - countl_zero(0) = -1 !!!
[[nodiscard]] constexpr int msb(Bitboard b)  noexcept {
    b |= 1;
    return 63 - std::countl_zero(b);    // Returns the number of consecutive 0 bits in the value of x, starting from the most significant bit ("left").
}

[[nodiscard]] constexpr int  Bcount(Bitboard b) noexcept { return std::popcount(b);}
[[nodiscard]] constexpr bool Bempty(Bitboard b) noexcept { return b == 0ULL; }
[[nodiscard]] constexpr bool Bsubset(Bitboard a, Bitboard b) noexcept { return( (a & b) == a); }

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

[[nodiscard]] constexpr Bitboard north(Bitboard b)  noexcept { return (b << 8); }
[[nodiscard]] constexpr Bitboard south(Bitboard b)  noexcept { return (b >> 8); }
[[nodiscard]] constexpr Bitboard east(Bitboard b)  noexcept  { return ((b << 1) & NOT_FILE_A_BB); }
[[nodiscard]] constexpr Bitboard west(Bitboard b)  noexcept  { return ((b >> 1) & NOT_FILE_H_BB); }
[[nodiscard]] constexpr Bitboard north_east(Bitboard b) noexcept { return((b << 9) & NOT_FILE_A_BB); }
[[nodiscard]] constexpr Bitboard south_east(Bitboard b) noexcept {  return((b >> 7) & NOT_FILE_A_BB); }
[[nodiscard]] constexpr Bitboard south_west(Bitboard b) noexcept { return((b >> 9) & NOT_FILE_H_BB); }
[[nodiscard]] constexpr Bitboard north_west(Bitboard b) noexcept { return((b << 7) & NOT_FILE_H_BB); }

[[nodiscard]] constexpr Bitboard north_north(Bitboard b)  noexcept { return (b << 16); }
[[nodiscard]] constexpr Bitboard south_south(Bitboard b)  noexcept { return (b >> 16); }

[[nodiscard]] constexpr Bitboard north_north_east(Bitboard b) noexcept { return((b << 17) & NOT_FILE_A_BB); }
[[nodiscard]] constexpr Bitboard north_north_west(Bitboard b) noexcept { return((b << 15) & NOT_FILE_H_BB); }

[[nodiscard]] constexpr Bitboard south_south_east(Bitboard b)  noexcept { return (b >> 15 & NOT_FILE_A_BB); }
[[nodiscard]] constexpr Bitboard south_south_west(Bitboard b)  noexcept { return (b >> 17 & NOT_FILE_H_BB); }

[[nodiscard]] constexpr Bitboard east_east_north(Bitboard b)  noexcept { return (b << 10 & NOT_FILE_AB_BB); }
[[nodiscard]] constexpr Bitboard east_east_south(Bitboard b)  noexcept { return (b >> 6  & NOT_FILE_AB_BB); }

[[nodiscard]] constexpr Bitboard west_west_north(Bitboard b)  noexcept { return (b << 6   & NOT_FILE_HG_BB); }
[[nodiscard]] constexpr Bitboard west_west_south(Bitboard b)  noexcept { return (b >> 10  & NOT_FILE_HG_BB); }

//! \brief Décale d'une case dans la direction donnée
template <Direction D>
[[nodiscard]] constexpr Bitboard ShiftBB(const Bitboard b)
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
    }
}

//! \brief Décale le bitboard verticalement
template <Direction D>
[[nodiscard]] constexpr Bitboard shift(const Bitboard b) { return (D == NORTH) ? b << 8 : b >> 8; }

//======================================
//! \brief  Impression d'un Bitboard
//--------------------------------------
inline void PrintBB(const Bitboard bb, const std::string& message)
{
    assert(bb);

    std::cout << "--------------------------- " << message << std::endl;
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
