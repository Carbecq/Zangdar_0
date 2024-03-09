#include "Square.h"
#include "Attacks.h"

namespace Attacks {

Bitboard ROOK_ATTACKS  [N_SQUARES][4096]{};
Bitboard BISHOP_ATTACKS[N_SQUARES][ 512]{};


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
        int sq = BB::pop_lsb(attack_mask);

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
    int sr = SQ::rank(sq);
    int sf = SQ::file(sq);

    // generate bishop atacks
    for (int r = sr + 1, f = sf + 1; r <= 7 && f <= 7; r++, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if (BB::get_bit(block, r * 8 + f))
            break;
    }

    for (int r = sr - 1, f = sf + 1; r >= 0 && f <= 7; r--, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if (BB::get_bit(block, r * 8 + f))
            break;
    }

    for (int r = sr + 1, f = sf - 1; r <= 7 && f >= 0; r++, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if (BB::get_bit(block, r * 8 + f))
            break;
    }

    for (int r = sr - 1, f = sf - 1; r >= 0 && f >= 0; r--, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if (BB::get_bit(block, r * 8 + f))
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
    int sr = SQ::rank(sq);
    int sf = SQ::file(sq);

    // generate rook attacks
    for (int r = sr + 1; r <= 7; r++) {
        attacks |= (1ULL << (r * 8 + sf));
        if (BB::get_bit(block, r * 8 + sf))
            break;
    }

    for (int r = sr - 1; r >= 0; r--) {
        attacks |= (1ULL << (r * 8 + sf));
        if (BB::get_bit(block, r * 8 + sf))
            break;
    }

    for (int f = sf + 1; f <= 7; f++) {
        attacks |= (1ULL << (sr * 8 + f));
        if (BB::get_bit(block, sr * 8 + f))
            break;
    }

    for (int f = sf - 1; f >= 0; f--) {
        attacks |= (1ULL << (sr * 8 + f));
        if (BB::get_bit(block, sr * 8 + f))
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

//================================================
//  Initialisation des attaques
//------------------------------------------------
void init_masks()
{
    init_bishop_attacks();
    init_rook_attacks();
}

} // namespace

