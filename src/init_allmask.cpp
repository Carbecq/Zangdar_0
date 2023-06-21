#include "Board.h"
#include "bitmask.h"
#include "MoveGen.h"

// isolated pawn masks [square]
Bitboard AdjacentFilesMask64[64] = {0};

// passed pawn masks [side][square]
Bitboard passed_pawn_mask[2][64] = {{0}};
Bitboard outpost_mask[2][64] = { {0} };
Bitboard rear_span_mask[2][64] = { {0} };
Bitboard backwards_mask[2][64] = { {0} };
Bitboard outer_kingring[64] = { 0 };
Bitboard inner_kingring[64] = { 0 };

/* Make a square from file & rank if inside the board */
int square_safe(const int f, const int r) {
    if (0 <= f && f < 8 && 0 <= r && r < 8) return Square::square(f, r);
    else return NO_SQUARE;
}

//================================================
//  Initialisation des bitmasks
//  Code provenant de Loki
//------------------------------------------------
void Board::init_bitmasks()
{
    Bitboard bitmask = 0;

    /*
    Passed pawn bitmasks
    */
    for (int sq = 0; sq < 64; sq++)
    {
        bitmask = 0;

        int r = sq / 8;
        int f = sq % 8;

        Bitboard flankmask = 0;

        flankmask |= FileMask8[f];

        if (f > FILE_A) {
            flankmask |= FileMask8[f - 1];
        }
        if (f < FILE_H) {
            flankmask |= FileMask8[f + 1];
        }

        // Create the bitmask for the white pawns
        for (int i = r + 1; i <= RANK_8; i++) {
            bitmask |= (flankmask & RankMask8[i]);
        }
        passed_pawn_mask[WHITE][sq] = bitmask;

        // Now do it for the black ones.
        bitmask = 0;
        for (int i = r - 1; i >= RANK_1; i--) {
            bitmask |= (flankmask & RankMask8[i]);
        }
        passed_pawn_mask[BLACK][sq] = bitmask;

        AdjacentFilesMask64[sq] = AdjacentFilesMask8[f];
    }

    /*
    Isolated bitmasks
    */
    for (int f = FILE_A; f <= FILE_H; f++)
    {
        // If a pawn is isolated, there are no pawns on the files directly next to it.
        // We shouln't include the file that the pawn is on itself, since we'd not be able to recognize it as isolated if it were doubled.
//        bitmask = ((f > FILE_A) ? FileMask8[f - 1] : 0) | ((f < FILE_H) ? FileMask8[f + 1] : 0);

 //       isolated_mask8[f] = AdjacentFilesMask8[f];
    }

    /*
    Outpost masks -> these can just be made by AND'ing the isolated bitmasks and passed pawn bitmasks.
    */
    for (int sq = 0; sq < 64; sq++)
    {
        outpost_mask[WHITE][sq] = (passed_pawn_mask[WHITE][sq] & AdjacentFilesMask64[sq]);
        outpost_mask[BLACK][sq] = (passed_pawn_mask[BLACK][sq] & AdjacentFilesMask64[sq]);
    }


    /*
    Rearspan bitmasks. The squares behind pawns.
    */
    for (int sq = 0; sq < 64; sq++)
    {
        rear_span_mask[WHITE][sq] = (passed_pawn_mask[BLACK][sq] & FileMask8[sq % 8]);
        rear_span_mask[BLACK][sq] = (passed_pawn_mask[WHITE][sq] & FileMask8[sq % 8]);
    }


    /*
    Backwards bitmasks. These are the squares on the current rank and all others behind it, on the adjacent files if a square.
    */
    for (int sq = 0; sq < 64; sq++) {

        backwards_mask[WHITE][sq] = (passed_pawn_mask[BLACK][sq] & ~FileMask8[sq % 8]);
        backwards_mask[BLACK][sq] = (passed_pawn_mask[WHITE][sq] & ~FileMask8[sq % 8]);

        if (sq % 8 != FILE_H)
        {
            backwards_mask[WHITE][sq] |= (uint64_t(1) << (sq + 1));
            backwards_mask[BLACK][sq] |= (uint64_t(1) << (sq + 1));
        }
        if (sq % 8 != FILE_A)
        {
            backwards_mask[WHITE][sq] |= (uint64_t(1) << (sq - 1));
            backwards_mask[BLACK][sq] |= (uint64_t(1) << (sq - 1));
        }
    }

    /*
    Inner king-rings. These are just the 8 squares next to the king.
    >> king_moves
    */

    /*
    Outer king-rings. These are just the 16 squares on the outside of the ring, that the king would be able to move to on an empty board.
    */
    for (int sq = 0; sq < 64; sq++)
    {
        Bitboard ring = 0;
        int rnk = sq / 8;
        int fl = sq % 8;

        for (int r = std::max(0, rnk - 2); r <= std::min(7, rnk + 2); r++) {
            for (int f = std::max(0, fl - 2); f <= std::min(7, fl + 2); f++) {
                ring |= (FileMask8[f] & RankMask8[r]);
            }
        }

        ring ^= MoveGen::king_moves(sq);
        ring ^= (uint64_t(1) << sq);

        outer_kingring[sq] = ring;
    }

    static const int king_dir[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

    static int d[64][64];

    for (int sq = 0; sq < 64; ++sq)
    {
        int f = Square::file(sq);
        int r = Square::rank(sq);

        for (int y = 0; y < 64; ++y)
            d[sq][y] = 0;

        // directions & between
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 1; j < 8; ++j)
            {
                int y = square_safe(f + king_dir[i][0] * j, r + king_dir[i][1] * j);
                if (y != NO_SQUARE)
                {
                    d[sq][y] = king_dir[i][0] + 8 * king_dir[i][1];
                    allmask[sq].direction[y] = abs(d[sq][y]);
                }
            }
        }

    }

}
