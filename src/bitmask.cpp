#include "bitmask.h"
#include "Attacks.h"
#include "Square.h"

Bitboard RankMask64[N_SQUARES] = {0};
Bitboard FileMask64[N_SQUARES] = {0};
Bitboard DiagonalMask64[N_SQUARES] = {0};
Bitboard AntiDiagonalMask64[N_SQUARES] = {0};
Bitboard AdjacentFilesMask64[N_SQUARES] = {0};

int      DISTANCE_BETWEEN[N_SQUARES][N_SQUARES];
Bitboard SQUARES_BETWEEN_MASK[64][64];

Bitboard RearSpanMask[N_COLORS][N_SQUARES] = { {0} };
Bitboard BackwardMask[N_COLORS][N_SQUARES] = { {0} };
Bitboard OutpostSquareMasks[N_COLORS][N_SQUARES];

Bitboard ForwardRanksMasks[N_COLORS][N_RANKS];
Bitboard ForwardFileMasks[N_COLORS][N_SQUARES];

Bitboard KingAreaMasks[N_COLORS][N_SQUARES];
int      KingPawnFileDistance[N_FILES][1 << N_FILES];

std::array<Mask, N_SQUARES> allmask;


//! \brief  Calcule la distance entre 2 cases
void calculate_distance_between_squares()
{
    for (int sq1 = A1; sq1 <= H8; ++sq1)
    {
        for (int sq2 = A1; sq2 <= H8; ++sq2)
        {
            int vertical   = abs(SQ::rank(sq1) - SQ::rank(sq2));
            int horizontal = abs(SQ::file(sq1) - SQ::file(sq2));
            DISTANCE_BETWEEN[sq1][sq2] = std::max(vertical, horizontal);
        }
    }
}

//! \brief initialise un bitboard constitué des cases entre 2 cases
//! alignées en diagonale, ou droite
void calculate_squares_between()
{
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j) {
            auto sq1 = i;
            auto sq2 = j;

            const auto dx = (SQ::file(sq2) - SQ::file(sq1));
            const auto dy = (SQ::rank(sq2) - SQ::rank(sq1));
            const auto adx = dx > 0 ? dx : -dx;
            const auto ady = dy > 0 ? dy : -dy;

            if (dx == 0 || dy == 0 || adx == ady)
            {
                Bitboard mask = 0ULL;
                while (sq1 != sq2) {
                    if (dx > 0) {
                        sq1 = SQ::east(sq1);
                    } else if (dx < 0) {
                        sq1 = SQ::west(sq1);
                    }
                    if (dy > 0) {
                        sq1 = SQ::north(sq1);
                    } else if (dy < 0) {
                        sq1 = SQ::south(sq1);
                    }
                    mask |= BB::sq2BB(sq1);
                }
                SQUARES_BETWEEN_MASK[i][j] = mask & ~BB::sq2BB(sq2);
            }
        }
    }


}

/* Make a square from file & rank if inside the board */
int square_safe(const int f, const int r) {
    if (0 <= f && f < 8 && 0 <= r && r < 8)
        return SQ::square(f, r);
    else
        return NO_SQUARE;
}

//================================================
//  Initialisation des bitmasks
//  Code provenant de Loki
//------------------------------------------------
void init_bitmasks()
{
    for (int sq = 0; sq < N_SQUARES; ++sq)
    {
        RankMask64[sq]         = RankMask8[SQ::rank(sq)];
        FileMask64[sq]         = FileMask8[SQ::file(sq)];
        DiagonalMask64[sq]     = DiagMask16[((sq >> 3) - (sq & 7)) & 15];
        AntiDiagonalMask64[sq] = ADiagMask16[((sq >> 3) + (sq & 7)) ^ 7];
    }

    calculate_squares_between();
    calculate_distance_between_squares();

 //   Bitboard bitmask = 0;

    /*
    Passed pawn bitmasks
    */
    for (int sq = 0; sq < N_SQUARES; sq++)
    {
 //       bitmask = 0;

 //       int r = SQ::rank(sq);
        int f = SQ::file(sq);

//       Bitboard flankmask = 0;

//        flankmask |= FileMask8[f];

        // if (f > FILE_A) {
        //     flankmask |= FileMask8[f - 1];
        // }
        // if (f < FILE_H) {
        //     flankmask |= FileMask8[f + 1];
        // }

        // Create the bitmask for the white pawns
        // for (int i = r + 1; i <= RANK_8; i++)
        // {
        //     bitmask |= (flankmask & RankMask8[i]);
        // }
        //       PassedPawnMask[WHITE][sq] = bitmask;

        // Now do it for the black ones.
        // bitmask = 0;
        // for (int i = r - 1; i >= RANK_1; i--)
        // {
        //     bitmask |= (flankmask & RankMask8[i]);
        // }
        // //        PassedPawnMask[BLACK][sq] = bitmask;

        AdjacentFilesMask64[sq] = AdjacentFilesMask8[f];
    }

    /*
    Rearspan bitmasks. The squares behind pawns.
    */
    for (int sq = 0; sq < 64; sq++)
    {
        RearSpanMask[WHITE][sq] = (PassedPawnMask[BLACK][sq] & FileMask8[sq % 8]);
        RearSpanMask[BLACK][sq] = (PassedPawnMask[WHITE][sq] & FileMask8[sq % 8]);
    }


    /*
    Backwards bitmasks. These are the squares on the current rank and all others behind it, on the adjacent files if a square.
    */
    for (int sq = 0; sq < 64; sq++) {

        BackwardMask[WHITE][sq] = (PassedPawnMask[BLACK][sq] & ~FileMask8[sq % 8]);
        BackwardMask[BLACK][sq] = (PassedPawnMask[WHITE][sq] & ~FileMask8[sq % 8]);

        if (sq % 8 != FILE_H)
        {
            BackwardMask[WHITE][sq] |= (uint64_t(1) << (sq + 1));
            BackwardMask[BLACK][sq] |= (uint64_t(1) << (sq + 1));
        }
        if (sq % 8 != FILE_A)
        {
            BackwardMask[WHITE][sq] |= (uint64_t(1) << (sq - 1));
            BackwardMask[BLACK][sq] |= (uint64_t(1) << (sq - 1));
        }
    }

    /*
    Inner king-rings. These are just the 8 squares next to the king.
    >> Attacks::king_moves
    */

    /*
    Outer king-rings. These are just the 16 squares on the outside of the ring, that the king would be able to move to on an empty board.
    */
    for (int sq = 0; sq < 64; sq++)
    {
        Bitboard ring = 0;
        int rnk = sq / 8;
        int fl = sq % 8;

        for (int r = std::max(0, rnk - 2); r <= std::min(7, rnk + 2); r++)
        {
            for (int f = std::max(0, fl - 2); f <= std::min(7, fl + 2); f++)
            {
                ring |= (FileMask8[f] & RankMask8[r]);
            }
        }

        ring ^= Attacks::king_moves(sq);
        ring ^= (uint64_t(1) << sq);

        //       outer_kingring[sq] = ring;
    }

    static const int king_dir[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

    static int d[64][64];

    for (int sq = 0; sq < 64; ++sq)
    {
        int f = SQ::file(sq);
        int r = SQ::rank(sq);

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

    // --->> Ethereal

    // Init a table to compute the distance between Pawns and Kings file-wise
    for (uint64_t mask = 0ull; mask <= 0xFF; mask++)
    {
        for (int file = 0; file < N_FILES; file++)
        {
            int ldist, rdist, dist;
            uint64_t left, right;

            // Look at only one side at a time by shifting off the other pawns
            left  = (0xFFull & (mask << (N_FILES - file - 1))) >> (N_FILES - file - 1);
            right = (mask >> file) << file;

            // Find closest Pawn on each side. If no pawn, use "max" distance
            ldist = left  ? file - BB::get_msb(left)  : N_FILES-1;
            rdist = right ? BB::get_lsb(right) - file : N_FILES-1;

            // Take the min distance, unless there are no pawns, then use 0
            dist = (left | right) ? std::min(ldist, rdist) : 0;
            KingPawnFileDistance[file][mask] = dist;
        }
    }

    // Init a table for the King Areas. Use the King's square, the King's target
    // squares, and the squares within the pawn shield. When on the A/H files, extend
    // the King Area to include an additional file, namely the C and F file respectively
    for (int sq = 0; sq < N_SQUARES; sq++)
    {
        KingAreaMasks[WHITE][sq] = Attacks::king_moves(sq) | (1ULL << sq) | (Attacks::king_moves(sq) << 8);
        KingAreaMasks[BLACK][sq] = Attacks::king_moves(sq) | (1ULL << sq) | (Attacks::king_moves(sq) >> 8);

        KingAreaMasks[WHITE][sq] |= SQ::file(sq) != 0 ? 0ULL : KingAreaMasks[WHITE][sq] << 1;
        KingAreaMasks[BLACK][sq] |= SQ::file(sq) != 0 ? 0ULL : KingAreaMasks[BLACK][sq] << 1;

        KingAreaMasks[WHITE][sq] |= SQ::file(sq) != 7 ? 0ULL : KingAreaMasks[WHITE][sq] >> 1;
        KingAreaMasks[BLACK][sq] |= SQ::file(sq) != 7 ? 0ULL : KingAreaMasks[BLACK][sq] >> 1;
    }

    // Init a table of bitmasks for the ranks at or above a given rank, by colour
    for (int rank = 0; rank < N_RANKS; rank++)
    {
        for (int i = rank; i < N_RANKS; i++)
            ForwardRanksMasks[WHITE][rank] |= RankMask8[i];
        ForwardRanksMasks[BLACK][rank] = ~ForwardRanksMasks[WHITE][rank] | RankMask8[rank];
    }

    // Init a table of bitmasks for the squares on a file above a given square, by colour
    for (int sq = 0; sq < N_SQUARES; sq++)
    {
        ForwardFileMasks[WHITE][sq] = FileMask64[sq] & ForwardRanksMasks[WHITE][SQ::rank(sq)];
        ForwardFileMasks[BLACK][sq] = FileMask64[sq] & ForwardRanksMasks[BLACK][SQ::rank(sq)];
    }

    // Init a table of bitmasks to check if a square is an outpost relative
    // to opposing pawns, such that no enemy pawn may attack the square with ease
    for (int colour = WHITE; colour <= BLACK; colour++)
        for (int sq = 0; sq < N_SQUARES; sq++)
            OutpostSquareMasks[colour][sq] = PassedPawnMask[colour][sq] & ~FileMask64[sq];



}
