#include "m42.h"


namespace M42 {
  uint64_t KnightAttacks[64];
  uint64_t KingAttacks[64];
  uint64_t PawnAttacks[2][64];
  uint64_t ThisAndNextSq[64];
  uint64_t PrevSquares[64];

  uint64_t RTables[0x16200]; // 708 kB
  uint64_t * RAttacks[64];
  uint64_t RMasks[64];
  uint64_t BTables[0x12C0]; // 37 kB
  uint64_t * BAttacks[64];
  uint64_t BMasks[64];

  // Initialize fancy magic bitboards
  void init_piece(bool rook, int sq)
  {
    uint64_t *Masks = rook ? RMasks : BMasks;
    const uint64_t * Magics = rook ? RMagics : BMagics;
    uint64_t ** Attacks = rook ? RAttacks : BAttacks;
    uint64_t(*calc_attacks)(int, uint64_t) =
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
    std::memset(Attacks[sq], 0, TableSize * sizeof(uint64_t));

    uint64_t occ = 0;
    do {
      uint32_t index = uint32_t(((occ & Masks[sq]) * Magics[sq]) >> Shift[sq]);
      assert((Attacks[sq][index] == 0)
        || (Attacks[sq][index] == calc_attacks(sq, occ)));
      Attacks[sq][index] = calc_attacks(sq, occ);
    } while (occ = next_subset(Masks[sq], occ));

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

      KingAttacks[sq] = calc_king_attacks(SquareMask64[sq]);
      KnightAttacks[sq] = calc_knight_attacks(SquareMask64[sq]);

      // Initialize pawn attacks
      PawnAttacks[0][sq] = calc_pawn_attacks<0>(SquareMask64[sq]);
      PawnAttacks[1][sq] = calc_pawn_attacks<1>(SquareMask64[sq]);
    }
  }
}
