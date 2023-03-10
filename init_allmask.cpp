#include "Board.h"
#include "bitmask.h"
#include "m42.h"

/* Make a square from file & rank if inside the board */
int square_safe(const int f, const int r) {
    if (0 <= f && f < 8 && 0 <= r && r < 8) return Square::square(f, r);
    else return NO_SQUARE;
}

//============================================
//! \brief  Initialisation des masques
//!
//! algorithme de Mperft
//--------------------------------------------
void Board::init_allmask()
{
    static const int king_dir[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

    static int d[64][64];

    for (int x = 0; x < 64; ++x)
    {
        int f = Square::file(x);
        int r = Square::rank(x);

        for (int y = 0; y < 64; ++y)
            d[x][y] = 0;

        // directions & between
        for (int i = 0; i < 8; ++i)
            for (int j = 1; j < 8; ++j)
            {
                int y = square_safe(f + king_dir[i][0] * j, r + king_dir[i][1] * j);
                if (y != NO_SQUARE)
                {
                    d[x][y] = king_dir[i][0] + 8 * king_dir[i][1];
                    allmask[x].direction[y] = abs(d[x][y]);
                }
            }

    }

}

