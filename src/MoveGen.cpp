#include "MoveGen.h"
#include <array>
#include <cassert>
#include <cstdint>
#include "defines.h"
#include "bitboard.h"
#include "Square.h"


namespace MoveGen {

Bitboard BETWEEN_SQS[64][64];

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

}
