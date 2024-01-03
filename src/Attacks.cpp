/*
 *  Code provenant de BBC
 */

#include <random>
#include "Attacks.h"
#include "bitmask.h"
#include "Square.h"

Bitboard RankMask64[64] = {0};
Bitboard FileMask64[64] = {0};
Bitboard DiagonalMask64[64] = {0};
Bitboard AntiDiagonalMask64[64] = {0};
Bitboard SquareMask64[64] = {0};

namespace Attacks {

Bitboard ROOK_ATTACKS  [64][4096]{};
Bitboard BISHOP_ATTACKS[64][ 512]{};
Bitboard BETWEEN_SQS[64][64];

//======================================================
//! \brief  set occupancies
//------------------------------------------------------
Bitboard set_occupancy(int index, int bits_in_mask, Bitboard attack_mask)
{
    // occupancy map
    Bitboard occupany = 0;

    // loop over the range of bits within attack mask
    for (int count = 0; count < bits_in_mask; count++)
    {
        // get LS1B index of attacks mask
        // pop LS1B in attack map
        int sq = next_square(attack_mask);

        // make sure occupancy is on board
        if (index & (1 << count))
            // populate occupancy map
            occupany |= (1ULL << sq);
    }

    // return occupancy map
    return occupany;
}

//======================================================
//! \brief  generate bishop attacks on the fly
//------------------------------------------------------
Bitboard bishop_attacks_on_the_fly(int sq, Bitboard block)
{
    // result attacks bitboard
    Bitboard attacks = 0;

    // init target rank & files
    int sr = Square::rank(sq);
    int sf = Square::file(sq);

    // generate bishop atacks
    for (int r = sr + 1, f = sf + 1; r <= 7 && f <= 7; r++, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if (get_bit(block, r * 8 + f))
            break;
    }

    for (int r = sr - 1, f = sf + 1; r >= 0 && f <= 7; r--, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if (get_bit(block, r * 8 + f))
            break;
    }

    for (int r = sr + 1, f = sf - 1; r <= 7 && f >= 0; r++, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if (get_bit(block, r * 8 + f))
            break;
    }

    for (int r = sr - 1, f = sf - 1; r >= 0 && f >= 0; r--, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if (get_bit(block, r * 8 + f))
            break;
    }

    // return attack map
    return attacks;
}

//======================================================
void init_bishop_attacks()
{
    // loop over 64 board squares
    for (int sq = 0; sq < 64; sq++)
    {
        // init current mask
        Bitboard mask = bishop_masks[sq];

        // init relevant occupancy bit count
        int bits      = bishop_relevant_bits[sq];

        // init occupancy indicies
        int indicies  = (1 << bits);

        // loop over occupancy indicies
        for (int index = 0; index < indicies; index++)
        {
            // init current occupancy variation
            Bitboard occupancy = set_occupancy(index, bits, mask);

#ifndef USE_PEXT
            // init magic index
            int idx                 = (occupancy * bishop_magics[sq]) >> (64 - bits);
            BISHOP_ATTACKS[sq][idx] = bishop_attacks_on_the_fly(sq, occupancy);
#else
            BISHOP_ATTACKS[sq][_pext_u64(occupancy, mask)] = bishop_attacks_on_the_fly(sq, occupancy);
#endif
        }
    }
}

//======================================================
//! \brief  generate rook attacks on the fly
//------------------------------------------------------
Bitboard rook_attacks_on_the_fly(int sq, Bitboard block)
{
    // result attacks bitboard
    Bitboard attacks = 0;

    // init target rank & files
    int sr = Square::rank(sq);
    int sf = Square::file(sq);

    // generate rook attacks
    for (int r = sr + 1; r <= 7; r++) {
        attacks |= (1ULL << (r * 8 + sf));
        if (get_bit(block, r * 8 + sf))
            break;
    }

    for (int r = sr - 1; r >= 0; r--) {
        attacks |= (1ULL << (r * 8 + sf));
        if (get_bit(block, r * 8 + sf))
            break;
    }

    for (int f = sf + 1; f <= 7; f++) {
        attacks |= (1ULL << (sr * 8 + f));
        if (get_bit(block, sr * 8 + f))
            break;
    }

    for (int f = sf - 1; f >= 0; f--) {
        attacks |= (1ULL << (sr * 8 + f));
        if (get_bit(block, sr * 8 + f))
            break;
    }

    // return attack map
    return attacks;
}

//======================================================
void init_rook_attacks()
{
    // loop over 64 board squares
    for (int sq = 0; sq < 64; sq++)
    {
        // init current mask
        Bitboard mask = rook_masks[sq];

        // init relevant occupancy bit count
        int bits      = rook_relevant_bits[sq];

        // init occupancy indicies
        int indicies  = (1 << bits);

        // loop over occupancy indicies
        for (int index = 0; index < indicies; index++)
        {
            // init current occupancy variation
            Bitboard occupancy = set_occupancy(index, bits, mask);

#ifndef USE_PEXT
            // init magic index
            int idx               = (occupancy * rook_magics[sq]) >> (64 - bits);
            ROOK_ATTACKS[sq][idx] = rook_attacks_on_the_fly(sq, occupancy);
#else
            ROOK_ATTACKS[sq][_pext_u64(occupancy, mask)]   = rook_attacks_on_the_fly(sq, occupancy);
#endif
        }
    }
}

void calculate_squares_between()
{
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j) {
            auto sq1 = i;
            auto sq2 = j;

            const auto dx = (Square::file(sq2) - Square::file(sq1));
            const auto dy = (Square::rank(sq2) - Square::rank(sq1));
            const auto adx = dx > 0 ? dx : -dx;
            const auto ady = dy > 0 ? dy : -dy;

            if (dx == 0 || dy == 0 || adx == ady)
            {
                Bitboard mask = 0ULL;
                while (sq1 != sq2) {
                    if (dx > 0) {
                        sq1 = Square::east(sq1);
                    } else if (dx < 0) {
                        sq1 = Square::west(sq1);
                    }
                    if (dy > 0) {
                        sq1 = Square::north(sq1);
                    } else if (dy < 0) {
                        sq1 = Square::south(sq1);
                    }
                    mask |= square_to_bit(sq1);
                }
                BETWEEN_SQS[i][j] = mask & ~square_to_bit(sq2);
            }
        }
    }
}

//======================================================
void init()
{
    for (int sq = 0; sq < 64; ++sq)
    {
        SquareMask64[sq]       = 1ULL << sq;

        RankMask64[sq]         = RankMask8[sq >> 3] & ~SquareMask64[sq];
        FileMask64[sq]         = FileMask8[sq & 7] & ~SquareMask64[sq];
        DiagonalMask64[sq]     = DiagMask16[((sq >> 3) - (sq & 7)) & 15] & ~SquareMask64[sq];
        AntiDiagonalMask64[sq] = ADiagMask16[((sq >> 3) + (sq & 7)) ^ 7] & ~SquareMask64[sq];
    }

    init_bishop_attacks();
    init_rook_attacks();
    calculate_squares_between();
}

} // namespace

