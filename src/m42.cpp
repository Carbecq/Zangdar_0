#include "m42.h"

Bitboard RankMask64[64] = {0};
Bitboard FileMask64[64] = {0};
Bitboard DiagonalMask64[64] = {0};
Bitboard AntiDiagonalMask64[64] = {0};
Bitboard SquareMask64[64] = {0};

namespace M42 {
  Bitboard ThisAndNextSq[64];
  Bitboard PrevSquares[64];

  Bitboard RTables[0x16200]; // 708 kB
  Bitboard * RAttacks[64];
  Bitboard RMasks[64];
  Bitboard BTables[0x12C0]; // 37 kB
  Bitboard * BAttacks[64];
  Bitboard BMasks[64];

  // Initialize fancy magic bitboards
  void init_piece(bool rook, int sq)
  {
    Bitboard *Masks = rook ? RMasks : BMasks;
    const Bitboard * Magics = rook ? RMagics : BMagics;
    Bitboard ** Attacks = rook ? RAttacks : BAttacks;
    Bitboard(*calc_attacks)(int, Bitboard) =
      rook ? calc_rook_attacks : calc_bishop_attacks;
    const unsigned * Shift = rook ? RShift : BShift;

    Masks[sq] = calc_attacks(sq, 0) & ~SquareMask64[sq];
    if ((sq & 7) != 0)
      Masks[sq] &= ~FileAMask;
    if ((sq & 7) != 7)
      Masks[sq] &= ~FileHMask;
    if ((sq >> 3) != 0)
      Masks[sq] &= ~0x00000000000000FFULL;  // Rank A
    if ((sq >> 3) != 7)
      Masks[sq] &= ~0xFF00000000000000ULL;  // Rank H

    const size_t TableSize = 1ULL << (64 - Shift[sq]);
    std::memset(Attacks[sq], 0, TableSize * sizeof(Bitboard));

    Bitboard occ = 0;
    do {
      uint32_t index = uint32_t(((occ & Masks[sq]) * Magics[sq]) >> Shift[sq]);
      assert((Attacks[sq][index] == 0)
        || (Attacks[sq][index] == calc_attacks(sq, occ)));
      Attacks[sq][index] = calc_attacks(sq, occ);
    } while ((occ = next_subset(Masks[sq], occ)));

    if (sq < 63)
      Attacks[sq + 1] = Attacks[sq] + TableSize;
  }

  void init()
  {
    int sq;

    for (sq = 0; sq < 64; ++sq) {
      SquareMask64[sq]  = 1ULL << sq;
      ThisAndNextSq[sq] = 3ULL << sq;
      PrevSquares[sq]   = ((1ULL << sq) - 1) + (sq == 0);

      RankMask64[sq]         = RankMask8[sq >> 3] & ~SquareMask64[sq];
      FileMask64[sq]         = FileMask8[sq & 7] & ~SquareMask64[sq];
      DiagonalMask64[sq]     = DiagMask16[((sq >> 3) - (sq & 7)) & 15] & ~SquareMask64[sq];
      AntiDiagonalMask64[sq] = ADiagMask16[((sq >> 3) + (sq & 7)) ^ 7] & ~SquareMask64[sq];
    }

    // Initialize all "fancy" magic bitboards
    RAttacks[0] = RTables;  // Set first offset
    BAttacks[0] = BTables;  // Set first offset

    for (sq = 0; sq < 64; ++sq) {
      init_piece(true, sq);
      init_piece(false, sq);
    }
  }
}
