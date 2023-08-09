#ifndef BITMASK_H
#define BITMASK_H

#include "defines.h"

constexpr auto FILE_A_BB = 0x0101010101010101ULL;
constexpr auto FILE_B_BB = 0x0202020202020202ULL;
constexpr auto FILE_C_BB = 0x0404040404040404ULL;
constexpr auto FILE_D_BB = 0x0808080808080808ULL;
constexpr auto FILE_E_BB = 0x1010101010101010ULL;
constexpr auto FILE_F_BB = 0x2020202020202020ULL;
constexpr auto FILE_G_BB = 0x4040404040404040ULL;
constexpr auto FILE_H_BB = 0x8080808080808080ULL;

constexpr auto RANK_1_BB = 0x00000000000000ffULL;
constexpr auto RANK_2_BB = 0x000000000000ff00ULL;
constexpr auto RANK_3_BB = 0x0000000000ff0000ULL;
constexpr auto RANK_4_BB = 0x00000000ff000000ULL;
constexpr auto RANK_5_BB = 0x000000ff00000000ULL;
constexpr auto RANK_6_BB = 0x0000ff0000000000ULL;
constexpr auto RANK_7_BB = 0x00ff000000000000ULL;
constexpr auto RANK_8_BB = 0xff00000000000000ULL;

constexpr Bitboard FileMask8[] = {FILE_A_BB, FILE_B_BB, FILE_C_BB, FILE_D_BB, FILE_E_BB, FILE_F_BB, FILE_G_BB, FILE_H_BB};
constexpr Bitboard RankMask8[] = {RANK_1_BB, RANK_2_BB, RANK_3_BB, RANK_4_BB, RANK_5_BB, RANK_6_BB, RANK_7_BB, RANK_8_BB};

constexpr Bitboard DiagMask16[] = {
  0x8040201008040201ULL, 0x4020100804020100ULL, 0x2010080402010000ULL,
  0x1008040201000000ULL, 0x0804020100000000ULL, 0x0402010000000000ULL,
  0x0201000000000000ULL, 0x0100000000000000ULL,
  0x0000000000000000ULL,
  0x0000000000000080ULL, 0x0000000000008040ULL, 0x0000000000804020ULL,
  0x0000000080402010ULL, 0x0000008040201008ULL, 0x0000804020100804ULL,
  0x0080402010080402ULL
};
constexpr Bitboard ADiagMask16[] = {
  0x0102040810204080ULL, 0x0001020408102040ULL, 0x0000010204081020ULL,
  0x0000000102040810ULL, 0x0000000001020408ULL, 0x0000000000010204ULL,
  0x0000000000000102ULL, 0x0000000000000001ULL,
  0x0000000000000000ULL,
  0x8000000000000000ULL, 0x4080000000000000ULL, 0x2040800000000000ULL,
  0x1020408000000000ULL, 0x0810204080000000ULL, 0x0408102040800000ULL,
  0x0204081020408000ULL
};


constexpr Bitboard AdjacentFilesMask8[8] =
{
 FILE_B_BB,
 FILE_A_BB | FILE_C_BB,
 FILE_B_BB | FILE_D_BB,
 FILE_C_BB | FILE_E_BB,
 FILE_D_BB | FILE_F_BB,
 FILE_E_BB | FILE_G_BB,
 FILE_F_BB | FILE_H_BB,
 FILE_G_BB
};

constexpr Bitboard LightSquares = 0x55aa55aa55aa55aaULL;
constexpr Bitboard DarkSquares  = 0xaa55aa55aa55aa55ULL;
constexpr Bitboard Empty        = 0x0000000000000000ULL;
constexpr Bitboard AllSquares   = 0xffffffffffffffffULL;
constexpr Bitboard Edge         = 0xff818181818181ffULL;

constexpr Bitboard Promotion_Rank[] = {RANK_8_BB, RANK_1_BB};
constexpr Bitboard Promoting_Rank[] = {RANK_7_BB, RANK_2_BB};
constexpr Bitboard Starting_Rank[]  = {RANK_2_BB, RANK_7_BB};

constexpr Bitboard NOT_FILE_A_BB = ~FILE_A_BB;
constexpr Bitboard NOT_FILE_H_BB = ~FILE_H_BB;

constexpr Bitboard NOT_FILE_HG_BB = 4557430888798830399ULL;
constexpr Bitboard NOT_FILE_AB_BB = 18229723555195321596ULL;

constexpr Bitboard OutpostRanks[2] = {
    RANK_4_BB | RANK_5_BB | RANK_6_BB,
    RANK_3_BB | RANK_4_BB | RANK_5_BB
};

extern Bitboard RankMask64[64];
extern Bitboard FileMask64[64];
extern Bitboard DiagonalMask64[64];
extern Bitboard AntiDiagonalMask64[64];
extern Bitboard SquareMask64[64];
extern Bitboard AdjacentFilesMask64[64];

#endif // BITMASK_H
