#include "defines.h"
#include "Board.h"

//  J'utilise les tables PeSTO

// Valeur des pièces
//  mg = middle game
//  eg = end game
const int mg_value[7] = { 0, 82, 337, 365, 477, 1025,  0};
const int eg_value[7] = { 0, 94, 281, 297, 512,  936,  0};

/* piece/sq tables */
/* values from Rofchade: http://www.talkchess.com/forum3/viewtopic.php?f=2&t=68311&start=19 */

/*
 *  A8              H8
 *
 *
 *  A1              H1
 *
 *
 *-------------------------- et moi je suis comme ça :
 *  A1              H1
 *
 *
 *  A8              H8
 *
 *
 *  >>> il faut intervertir blanc et noir
 */


const int mg_black_pawn_table[BOARD_SIZE] = {
    //    a    b    c    d    e    f   g    h

    0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   // 1
    98, 134,  61,  95,  68, 126, 34, -11,   0,   0,   0,   0,   0,   0,  0,   0,   // 2
    -6,   7,  26,  31,  65,  56, 25, -20,   0,   0,   0,   0,   0,   0,  0,   0,   // 3
    -14,  13,   6,  21,  23,  12, 17, -23,   0,   0,   0,   0,   0,   0,  0,   0,   // 4
    -27,  -2,  -5,  12,  17,   6, 10, -25,   0,   0,   0,   0,   0,   0,  0,   0,   // 5
    -26,  -4,  -4, -10,   3,   3, 33, -12,   0,   0,   0,   0,   0,   0,  0,   0,   // 6
    -35,  -1, -20, -23, -15,  24, 38, -22,   0,   0,   0,   0,   0,   0,  0,   0,   // 7
    0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,   0,  0,   0    // 8
};
const int mg_white_pawn_table[BOARD_SIZE] = {
    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    -35,    -1,   -20,   -23,   -15,    24,    38,   -22,     0,     0,     0,     0,     0,     0,     0,     0,
    -26,    -4,    -4,   -10,     3,     3,    33,   -12,     0,     0,     0,     0,     0,     0,     0,     0,
    -27,    -2,    -5,    12,    17,     6,    10,   -25,     0,     0,     0,     0,     0,     0,     0,     0,
    -14,    13,     6,    21,    23,    12,    17,   -23,     0,     0,     0,     0,     0,     0,     0,     0,
    -6,     7,    26,    31,    65,    56,    25,   -20,     0,     0,     0,     0,     0,     0,     0,     0,
    98,   134,    61,    95,    68,   126,    34,   -11,     0,     0,     0,     0,     0,     0,     0,     0,
    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
};


const int eg_black_pawn_table[BOARD_SIZE] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,   0,   0,   0,   0,   0,   0,  0,   0,
    94, 100,  85,  67,  56,  53,  82,  84,   0,   0,   0,   0,   0,   0,  0,   0,
    32,  24,  13,   5,  -2,   4,  17,  17,   0,   0,   0,   0,   0,   0,  0,   0,
    13,   9,  -3,  -7,  -7,  -8,   3,  -1,   0,   0,   0,   0,   0,   0,  0,   0,
    4,   7,  -6,   1,   0,  -5,  -1,  -8,   0,   0,   0,   0,   0,   0,  0,   0,
    13,   8,   8,  10,  13,   0,   2,  -7,   0,   0,   0,   0,   0,   0,  0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,
};
const int eg_white_pawn_table[BOARD_SIZE] = {
    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    13,     8,     8,    10,    13,     0,     2,    -7,     0,     0,     0,     0,     0,     0,     0,     0,
    4,     7,    -6,     1,     0,    -5,    -1,    -8,     0,     0,     0,     0,     0,     0,     0,     0,
    13,     9,    -3,    -7,    -7,    -8,     3,    -1,     0,     0,     0,     0,     0,     0,     0,     0,
    32,    24,    13,     5,    -2,     4,    17,    17,     0,     0,     0,     0,     0,     0,     0,     0,
    94,   100,    85,    67,    56,    53,    82,    84,     0,     0,     0,     0,     0,     0,     0,     0,
    178,   173,   158,   134,   147,   132,   165,   187,     0,     0,     0,     0,     0,     0,     0,     0,
    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
};



const int mg_black_knight_table[BOARD_SIZE] = {
    -167, -89, -34, -49,  61, -97, -15, -107,   0,   0,   0,   0,   0,   0,  0,   0,
    -73, -41,  72,  36,  23,  62,   7,  -17,   0,   0,   0,   0,   0,   0,  0,   0,
    -47,  60,  37,  65,  84, 129,  73,   44,   0,   0,   0,   0,   0,   0,  0,   0,
    -9,  17,  19,  53,  37,  69,  18,   22,   0,   0,   0,   0,   0,   0,  0,   0,
    -13,   4,  16,  13,  28,  19,  21,   -8,   0,   0,   0,   0,   0,   0,  0,   0,
    -23,  -9,  12,  10,  19,  17,  25,  -16,   0,   0,   0,   0,   0,   0,  0,   0,
    -29, -53, -12,  -3,  -1,  18, -14,  -19,   0,   0,   0,   0,   0,   0,  0,   0,
    -105, -21, -58, -33, -17, -28, -19,  -23,   0,   0,   0,   0,   0,   0,  0,   0,
};
const int mg_white_knight_table[BOARD_SIZE] = {
    -105,   -21,   -58,   -33,   -17,   -28,   -19,   -23,     0,     0,     0,     0,     0,     0,     0,     0,
    -29,   -53,   -12,    -3,    -1,    18,   -14,   -19,     0,     0,     0,     0,     0,     0,     0,     0,
    -23,    -9,    12,    10,    19,    17,    25,   -16,     0,     0,     0,     0,     0,     0,     0,     0,
    -13,     4,    16,    13,    28,    19,    21,    -8,     0,     0,     0,     0,     0,     0,     0,     0,
    -9,    17,    19,    53,    37,    69,    18,    22,     0,     0,     0,     0,     0,     0,     0,     0,
    -47,    60,    37,    65,    84,   129,    73,    44,     0,     0,     0,     0,     0,     0,     0,     0,
    -73,   -41,    72,    36,    23,    62,     7,   -17,     0,     0,     0,     0,     0,     0,     0,     0,
    -167,   -89,   -34,   -49,    61,   -97,   -15,  -107,     0,     0,     0,     0,     0,     0,     0,     0,
};


const int eg_black_knight_table[BOARD_SIZE] = {
    -58, -38, -13, -28, -31, -27, -63, -99,   0,   0,   0,   0,   0,   0,  0,   0,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,   0,   0,   0,   0,   0,   0,  0,   0,
    -24, -20,  10,   9,  -1,  -9, -19, -41,   0,   0,   0,   0,   0,   0,  0,   0,
    -17,   3,  22,  22,  22,  11,   8, -18,   0,   0,   0,   0,   0,   0,  0,   0,
    -18,  -6,  16,  25,  16,  17,   4, -18,   0,   0,   0,   0,   0,   0,  0,   0,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,   0,   0,   0,   0,   0,   0,  0,   0,
    -42, -20, -10,  -5,  -2, -20, -23, -44,   0,   0,   0,   0,   0,   0,  0,   0,
    -29, -51, -23, -15, -22, -18, -50, -64,   0,   0,   0,   0,   0,   0,  0,   0,
};
const int eg_white_knight_table[BOARD_SIZE] = {
    -29,   -51,   -23,   -15,   -22,   -18,   -50,   -64,     0,     0,     0,     0,     0,     0,     0,     0,
    -42,   -20,   -10,    -5,    -2,   -20,   -23,   -44,     0,     0,     0,     0,     0,     0,     0,     0,
    -23,    -3,    -1,    15,    10,    -3,   -20,   -22,     0,     0,     0,     0,     0,     0,     0,     0,
    -18,    -6,    16,    25,    16,    17,     4,   -18,     0,     0,     0,     0,     0,     0,     0,     0,
    -17,     3,    22,    22,    22,    11,     8,   -18,     0,     0,     0,     0,     0,     0,     0,     0,
    -24,   -20,    10,     9,    -1,    -9,   -19,   -41,     0,     0,     0,     0,     0,     0,     0,     0,
    -25,    -8,   -25,    -2,    -9,   -25,   -24,   -52,     0,     0,     0,     0,     0,     0,     0,     0,
    -58,   -38,   -13,   -28,   -31,   -27,   -63,   -99,     0,     0,     0,     0,     0,     0,     0,     0,
};


const int mg_black_bishop_table[BOARD_SIZE] = {
    -29,   4, -82, -37, -25, -42,   7,  -8,   0,   0,   0,   0,   0,   0,  0,   0,
    -26,  16, -18, -13,  30,  59,  18, -47,   0,   0,   0,   0,   0,   0,  0,   0,
    -16,  37,  43,  40,  35,  50,  37,  -2,   0,   0,   0,   0,   0,   0,  0,   0,
    -4,   5,  19,  50,  37,  37,   7,  -2,   0,   0,   0,   0,   0,   0,  0,   0,
    -6,  13,  13,  26,  34,  12,  10,   4,   0,   0,   0,   0,   0,   0,  0,   0,
    0,  15,  15,  15,  14,  27,  18,  10,   0,   0,   0,   0,   0,   0,  0,   0,
    4,  15,  16,   0,   7,  21,  33,   1,   0,   0,   0,   0,   0,   0,  0,   0,
    -33,  -3, -14, -21, -13, -12, -39, -21,   0,   0,   0,   0,   0,   0,  0,   0,
};
const int mg_white_bishop_table[BOARD_SIZE] = {
    -33,    -3,   -14,   -21,   -13,   -12,   -39,   -21,     0,     0,     0,     0,     0,     0,     0,     0,
    4,    15,    16,     0,     7,    21,    33,     1,     0,     0,     0,     0,     0,     0,     0,     0,
    0,    15,    15,    15,    14,    27,    18,    10,     0,     0,     0,     0,     0,     0,     0,     0,
    -6,    13,    13,    26,    34,    12,    10,     4,     0,     0,     0,     0,     0,     0,     0,     0,
    -4,     5,    19,    50,    37,    37,     7,    -2,     0,     0,     0,     0,     0,     0,     0,     0,
    -16,    37,    43,    40,    35,    50,    37,    -2,     0,     0,     0,     0,     0,     0,     0,     0,
    -26,    16,   -18,   -13,    30,    59,    18,   -47,     0,     0,     0,     0,     0,     0,     0,     0,
    -29,     4,   -82,   -37,   -25,   -42,     7,    -8,     0,     0,     0,     0,     0,     0,     0,     0,
};


const int eg_black_bishop_table[BOARD_SIZE] = {
    -14, -21, -11,  -8, -7,  -9, -17, -24,   0,   0,   0,   0,   0,   0,  0,   0,
    -8,  -4,   7, -12, -3, -13,  -4, -14,   0,   0,   0,   0,   0,   0,  0,   0,
    2,  -8,   0,  -1, -2,   6,   0,   4,   0,   0,   0,   0,   0,   0,  0,   0,
    -3,   9,  12,   9, 14,  10,   3,   2,   0,   0,   0,   0,   0,   0,  0,   0,
    -6,   3,  13,  19,  7,  10,  -3,  -9,   0,   0,   0,   0,   0,   0,  0,   0,
    -12,  -3,   8,  10, 13,   3,  -7, -15,   0,   0,   0,   0,   0,   0,  0,   0,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,   0,   0,   0,   0,   0,   0,  0,   0,
    -23,  -9, -23,  -5, -9, -16,  -5, -17,   0,   0,   0,   0,   0,   0,  0,   0,
};
const int eg_white_bishop_table[BOARD_SIZE] = {
    -23,    -9,   -23,    -5,    -9,   -16,    -5,   -17,     0,     0,     0,     0,     0,     0,     0,     0,
    -14,   -18,    -7,    -1,     4,    -9,   -15,   -27,     0,     0,     0,     0,     0,     0,     0,     0,
    -12,    -3,     8,    10,    13,     3,    -7,   -15,     0,     0,     0,     0,     0,     0,     0,     0,
    -6,     3,    13,    19,     7,    10,    -3,    -9,     0,     0,     0,     0,     0,     0,     0,     0,
    -3,     9,    12,     9,    14,    10,     3,     2,     0,     0,     0,     0,     0,     0,     0,     0,
    2,    -8,     0,    -1,    -2,     6,     0,     4,     0,     0,     0,     0,     0,     0,     0,     0,
    -8,    -4,     7,   -12,    -3,   -13,    -4,   -14,     0,     0,     0,     0,     0,     0,     0,     0,
    -14,   -21,   -11,    -8,    -7,    -9,   -17,   -24,     0,     0,     0,     0,     0,     0,     0,     0,
};

const int mg_black_rook_table[BOARD_SIZE] = {
    32,  42,  32,  51, 63,  9,  31,  43,   0,   0,   0,   0,   0,   0,  0,   0,
    27,  32,  58,  62, 80, 67,  26,  44,   0,   0,   0,   0,   0,   0,  0,   0,
    -5,  19,  26,  36, 17, 45,  61,  16,   0,   0,   0,   0,   0,   0,  0,   0,
    -24, -11,   7,  26, 24, 35,  -8, -20,   0,   0,   0,   0,   0,   0,  0,   0,
    -36, -26, -12,  -1,  9, -7,   6, -23,   0,   0,   0,   0,   0,   0,  0,   0,
    -45, -25, -16, -17,  3,  0,  -5, -33,   0,   0,   0,   0,   0,   0,  0,   0,
    -44, -16, -20,  -9, -1, 11,  -6, -71,   0,   0,   0,   0,   0,   0,  0,   0,
    -19, -13,   1,  17, 16,  7, -37, -26,   0,   0,   0,   0,   0,   0,  0,   0,
};
const int mg_white_rook_table[BOARD_SIZE] = {
    -19,   -13,     1,    17,    16,     7,   -37,   -26,     0,     0,     0,     0,     0,     0,     0,     0,
    -44,   -16,   -20,    -9,    -1,    11,    -6,   -71,     0,     0,     0,     0,     0,     0,     0,     0,
    -45,   -25,   -16,   -17,     3,     0,    -5,   -33,     0,     0,     0,     0,     0,     0,     0,     0,
    -36,   -26,   -12,    -1,     9,    -7,     6,   -23,     0,     0,     0,     0,     0,     0,     0,     0,
    -24,   -11,     7,    26,    24,    35,    -8,   -20,     0,     0,     0,     0,     0,     0,     0,     0,
    -5,    19,    26,    36,    17,    45,    61,    16,     0,     0,     0,     0,     0,     0,     0,     0,
    27,    32,    58,    62,    80,    67,    26,    44,     0,     0,     0,     0,     0,     0,     0,     0,
    32,    42,    32,    51,    63,     9,    31,    43,     0,     0,     0,     0,     0,     0,     0,     0,
};


const int eg_black_rook_table[BOARD_SIZE] = {
    13, 10, 18, 15, 12,  12,   8,   5,   0,   0,   0,   0,   0,   0,  0,   0,
    11, 13, 13, 11, -3,   3,   8,   3,   0,   0,   0,   0,   0,   0,  0,   0,
    7,  7,  7,  5,  4,  -3,  -5,  -3,   0,   0,   0,   0,   0,   0,  0,   0,
    4,  3, 13,  1,  2,   1,  -1,   2,   0,   0,   0,   0,   0,   0,  0,   0,
    3,  5,  8,  4, -5,  -6,  -8, -11,   0,   0,   0,   0,   0,   0,  0,   0,
    -4,  0, -5, -1, -7, -12,  -8, -16,   0,   0,   0,   0,   0,   0,  0,   0,
    -6, -6,  0,  2, -9,  -9, -11,  -3,   0,   0,   0,   0,   0,   0,  0,   0,
    -9,  2,  3, -1, -5, -13,   4, -20,   0,   0,   0,   0,   0,   0,  0,   0,
};
const int eg_white_rook_table[BOARD_SIZE] = {
    -9,     2,     3,    -1,    -5,   -13,     4,   -20,     0,     0,     0,     0,     0,     0,     0,     0,
    -6,    -6,     0,     2,    -9,    -9,   -11,    -3,     0,     0,     0,     0,     0,     0,     0,     0,
    -4,     0,    -5,    -1,    -7,   -12,    -8,   -16,     0,     0,     0,     0,     0,     0,     0,     0,
    3,     5,     8,     4,    -5,    -6,    -8,   -11,     0,     0,     0,     0,     0,     0,     0,     0,
    4,     3,    13,     1,     2,     1,    -1,     2,     0,     0,     0,     0,     0,     0,     0,     0,
    7,     7,     7,     5,     4,    -3,    -5,    -3,     0,     0,     0,     0,     0,     0,     0,     0,
    11,    13,    13,    11,    -3,     3,     8,     3,     0,     0,     0,     0,     0,     0,     0,     0,
    13,    10,    18,    15,    12,    12,     8,     5,     0,     0,     0,     0,     0,     0,     0,     0,
};

const int mg_black_queen_table[BOARD_SIZE] = {
    -28,   0,  29,  12,  59,  44,  43,  45,   0,   0,   0,   0,   0,   0,  0,   0,
    -24, -39,  -5,   1, -16,  57,  28,  54,   0,   0,   0,   0,   0,   0,  0,   0,
    -13, -17,   7,   8,  29,  56,  47,  57,   0,   0,   0,   0,   0,   0,  0,   0,
    -27, -27, -16, -16,  -1,  17,  -2,   1,   0,   0,   0,   0,   0,   0,  0,   0,
    -9, -26,  -9, -10,  -2,  -4,   3,  -3,   0,   0,   0,   0,   0,   0,  0,   0,
    -14,   2, -11,  -2,  -5,   2,  14,   5,   0,   0,   0,   0,   0,   0,  0,   0,
    -35,  -8,  11,   2,   8,  15,  -3,   1,   0,   0,   0,   0,   0,   0,  0,   0,
    -1, -18,  -9,  10, -15, -25, -31, -50,   0,   0,   0,   0,   0,   0,  0,   0,
};
const int mg_white_queen_table[BOARD_SIZE] = {
    -1,   -18,    -9,    10,   -15,   -25,   -31,   -50,     0,     0,     0,     0,     0,     0,     0,     0,
    -35,    -8,    11,     2,     8,    15,    -3,     1,     0,     0,     0,     0,     0,     0,     0,     0,
    -14,     2,   -11,    -2,    -5,     2,    14,     5,     0,     0,     0,     0,     0,     0,     0,     0,
    -9,   -26,    -9,   -10,    -2,    -4,     3,    -3,     0,     0,     0,     0,     0,     0,     0,     0,
    -27,   -27,   -16,   -16,    -1,    17,    -2,     1,     0,     0,     0,     0,     0,     0,     0,     0,
    -13,   -17,     7,     8,    29,    56,    47,    57,     0,     0,     0,     0,     0,     0,     0,     0,
    -24,   -39,    -5,     1,   -16,    57,    28,    54,     0,     0,     0,     0,     0,     0,     0,     0,
    -28,     0,    29,    12,    59,    44,    43,    45,     0,     0,     0,     0,     0,     0,     0,     0,
};


const int eg_black_queen_table[BOARD_SIZE] = {
    -9,  22,  22,  27,  27,  19,  10,  20,   0,   0,   0,   0,   0,   0,  0,   0,
    -17,  20,  32,  41,  58,  25,  30,   0,   0,   0,   0,   0,   0,   0,  0,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,   0,   0,   0,   0,   0,   0,  0,   0,
    3,  22,  24,  45,  57,  40,  57,  36,   0,   0,   0,   0,   0,   0,  0,   0,
    -18,  28,  19,  47,  31,  34,  39,  23,   0,   0,   0,   0,   0,   0,  0,   0,
    -16, -27,  15,   6,   9,  17,  10,   5,   0,   0,   0,   0,   0,   0,  0,   0,
    -22, -23, -30, -16, -16, -23, -36, -32,   0,   0,   0,   0,   0,   0,  0,   0,
    -33, -28, -22, -43,  -5, -32, -20, -41,   0,   0,   0,   0,   0,   0,  0,   0,
};
const int eg_white_queen_table[BOARD_SIZE] = {
    -33,   -28,   -22,   -43,    -5,   -32,   -20,   -41,     0,     0,     0,     0,     0,     0,     0,     0,
    -22,   -23,   -30,   -16,   -16,   -23,   -36,   -32,     0,     0,     0,     0,     0,     0,     0,     0,
    -16,   -27,    15,     6,     9,    17,    10,     5,     0,     0,     0,     0,     0,     0,     0,     0,
    -18,    28,    19,    47,    31,    34,    39,    23,     0,     0,     0,     0,     0,     0,     0,     0,
    3,    22,    24,    45,    57,    40,    57,    36,     0,     0,     0,     0,     0,     0,     0,     0,
    -20,     6,     9,    49,    47,    35,    19,     9,     0,     0,     0,     0,     0,     0,     0,     0,
    -17,    20,    32,    41,    58,    25,    30,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    -9,    22,    22,    27,    27,    19,    10,    20,     0,     0,     0,     0,     0,     0,     0,     0,
};

const int mg_black_king_table[BOARD_SIZE] = {
    -65,  23,  16, -15, -56, -34,   2,  13,   0,   0,   0,   0,   0,   0,  0,   0,
    29,  -1, -20,  -7,  -8,  -4, -38, -29,   0,   0,   0,   0,   0,   0,  0,   0,
    -9,  24,   2, -16, -20,   6,  22, -22,   0,   0,   0,   0,   0,   0,  0,   0,
    -17, -20, -12, -27, -30, -25, -14, -36,   0,   0,   0,   0,   0,   0,  0,   0,
    -49,  -1, -27, -39, -46, -44, -33, -51,   0,   0,   0,   0,   0,   0,  0,   0,
    -14, -14, -22, -46, -44, -30, -15, -27,   0,   0,   0,   0,   0,   0,  0,   0,
    1,   7,  -8, -64, -43, -16,   9,   8,   0,   0,   0,   0,   0,   0,  0,   0,
    -15,  36,  12, -54,   8, -28,  24,  14,   0,   0,   0,   0,   0,   0,  0,   0,
};
const int mg_white_king_table[BOARD_SIZE] = {
    -15,    36,    12,   -54,     8,   -28,    24,    14,     0,     0,     0,     0,     0,     0,     0,     0,
    1,     7,    -8,   -64,   -43,   -16,     9,     8,     0,     0,     0,     0,     0,     0,     0,     0,
    -14,   -14,   -22,   -46,   -44,   -30,   -15,   -27,     0,     0,     0,     0,     0,     0,     0,     0,
    -49,    -1,   -27,   -39,   -46,   -44,   -33,   -51,     0,     0,     0,     0,     0,     0,     0,     0,
    -17,   -20,   -12,   -27,   -30,   -25,   -14,   -36,     0,     0,     0,     0,     0,     0,     0,     0,
    -9,    24,     2,   -16,   -20,     6,    22,   -22,     0,     0,     0,     0,     0,     0,     0,     0,
    29,    -1,   -20,    -7,    -8,    -4,   -38,   -29,     0,     0,     0,     0,     0,     0,     0,     0,
    -65,    23,    16,   -15,   -56,   -34,     2,    13,     0,     0,     0,     0,     0,     0,     0,     0,
};

const int eg_black_king_table[BOARD_SIZE] = {
    -74, -35, -18, -18, -11,  15,   4, -17,   0,   0,   0,   0,   0,   0,  0,   0,
    -12,  17,  14,  17,  17,  38,  23,  11,   0,   0,   0,   0,   0,   0,  0,   0,
    10,  17,  23,  15,  20,  45,  44,  13,   0,   0,   0,   0,   0,   0,  0,   0,
    -8,  22,  24,  27,  26,  33,  26,   3,   0,   0,   0,   0,   0,   0,  0,   0,
    -18,  -4,  21,  24,  27,  23,   9, -11,   0,   0,   0,   0,   0,   0,  0,   0,
    -19,  -3,  11,  21,  23,  16,   7,  -9,   0,   0,   0,   0,   0,   0,  0,   0,
    -27, -11,   4,  13,  14,   4,  -5, -17,   0,   0,   0,   0,   0,   0,  0,   0,
    -53, -34, -21, -11, -28, -14, -24, -43,   0,   0,   0,   0,   0,   0,  0,   0,
};
const int eg_white_king_table[BOARD_SIZE] = {
    -53,   -34,   -21,   -11,   -28,   -14,   -24,   -43,     0,     0,     0,     0,     0,     0,     0,     0,
    -27,   -11,     4,    13,    14,     4,    -5,   -17,     0,     0,     0,     0,     0,     0,     0,     0,
    -19,    -3,    11,    21,    23,    16,     7,    -9,     0,     0,     0,     0,     0,     0,     0,     0,
    -18,    -4,    21,    24,    27,    23,     9,   -11,     0,     0,     0,     0,     0,     0,     0,     0,
    -8,    22,    24,    27,    26,    33,    26,     3,     0,     0,     0,     0,     0,     0,     0,     0,
    10,    17,    23,    15,    20,    45,    44,    13,     0,     0,     0,     0,     0,     0,     0,     0,
    -12,    17,    14,    17,    17,    38,    23,    11,     0,     0,     0,     0,     0,     0,     0,     0,
    -74,   -35,   -18,   -18,   -11,    15,     4,   -17,     0,     0,     0,     0,     0,     0,     0,     0,
};


void Board::flip()
{
//    int sq;
//    FILE* fp = fopen("flip.txt", "w");

//    fprintf(fp, "const int eg_white_king_table[BOARD_SIZE] = {\n");

//    for (int i = 7; i>=0; i--)
//    {
//        for (int j=0; j<16; j++)
//        {
//            sq = i*16 + j;

//            fprintf(fp, "%5d, ", eg_black_king_table[sq]);
//        }
//        fprintf(fp, "\n");
//    }
//    fprintf(fp, "};\n");
//    fclose(fp);
}

// Voir Sjeng : neval.c

int Board::evaluate(Color side)
{
    Square  sq;
//    int file;
    //    int rank;
    //    char cfile, crank;

    int mg[2];
    int eg[2];
    int gamePhase = 0;
//    int nbr_bishop = 0;
//    int nbr;

    mg[WHITE] = 0;
    mg[BLACK] = 0;
    eg[WHITE] = 0;
    eg[BLACK] = 0;

    // nullité
    // voir le code de Sjeng, qui comporte un test s'il reste des pions
    if(MaterialDraw() == true)
        return 0;

    // evaluate each piece
    for (auto & p : pieces[WHITE])
    {
        if (p.dead() == false)
        {
            mg[WHITE] += mg_value[p.type()];
            eg[WHITE] += eg_value[p.type()];

            sq = p.square();

            switch(p.type())
            {
            case PAWN:
                mg[WHITE] += mg_white_pawn_table[sq];
                eg[WHITE] += eg_white_pawn_table[sq];
                break;
            case KNIGHT:
                mg[WHITE] += mg_white_knight_table[sq];
                eg[WHITE] += eg_white_knight_table[sq];
                gamePhase += 1;
                break;
            case BISHOP:
                mg[WHITE] += mg_white_bishop_table[sq];
                eg[WHITE] += eg_white_bishop_table[sq];
//                nbr_bishop++;
                gamePhase += 1;
                break;
            case ROOK:
                mg[WHITE] += mg_white_rook_table[sq];
                eg[WHITE] += eg_white_rook_table[sq];
                gamePhase += 2;
                break;
            case QUEEN:
                mg[WHITE] += mg_white_queen_table[sq];
                eg[WHITE] += eg_white_queen_table[sq];
                gamePhase += 4;
                break;
            case KING:
                mg[WHITE] += mg_white_king_table[sq];
                eg[WHITE] += eg_white_king_table[sq];
                break;
            default:
                break;
            }
        }
    }

//    if(nbr_bishop >= 2)
//        mg[WHITE] += BishopPair;

//    nbr_bishop   = 0;

    for (auto & p : pieces[BLACK])
    {
        if (p.dead() == false)
        {
            sq = p.square();

            mg[BLACK] += mg_value[p.type()];
            eg[BLACK] += eg_value[p.type()];

            switch(p.type())
            {
            case PAWN:
                mg[BLACK] += mg_black_pawn_table[sq];
                eg[BLACK] += eg_black_pawn_table[sq];
                break;
            case KNIGHT:
                mg[BLACK] += mg_black_knight_table[sq];
                eg[BLACK] += eg_black_knight_table[sq];
                gamePhase += 1;
                break;
            case BISHOP:
                mg[BLACK] += mg_black_bishop_table[sq];
                eg[BLACK] += eg_black_bishop_table[sq];
//                nbr_bishop++;
                gamePhase += 1;
                break;
            case ROOK:
                mg[BLACK] += mg_black_rook_table[sq];
                eg[BLACK] += eg_black_rook_table[sq];
                gamePhase += 2;
                break;
            case QUEEN:
                mg[BLACK] += mg_black_queen_table[sq];
                eg[BLACK] += eg_black_queen_table[sq];
                gamePhase += 4;
                break;
            case KING:
                mg[BLACK] += mg_black_king_table[sq];
                eg[BLACK] += eg_black_king_table[sq];
                break;
            default:
                break;
            }
        }
    }

//    if(nbr_bishop >= 2)
//        mg[BLACK] += BishopPair;

    // scale score according to game phase

    int mgScore = mg[WHITE] - mg[BLACK];
    int egScore = eg[WHITE] - eg[BLACK];
    int mgPhase = gamePhase;
    if (mgPhase > 24)
        mgPhase = 24; // in case of early promotion
    int egPhase = 24 - mgPhase;

    I32 score = (mgScore * mgPhase + egScore * egPhase) / 24;

    // return score relative to the side to move
    if (side == WHITE)
        return score;
    else
        return -score;
}

// Code de Vice (chap 82)
//  >> vient de Sjeng 11.2 (draw.c) et neval.c , ligne 588
//  >> qui vient de Faile
//  8/6R1/2k5/6P1/8/8/4nP2/6K1 w - - 1 41   : position vue dns la vidéo de Vice, à qui sert-elle ?

//===============================================================
//! \brief  Détermine si la position est nulle
//!         par manque de matériel
//!
//! \param[in]  pos     position de recherche
//! \return             nullité
//---------------------------------------------------------------
bool Board::MaterialDraw() const
{
    //    ASSERT(CheckBoard(pos));

    int nbr_piece[2][7];

    for (int c=0; c<2; c++)
        for (int t=0; t<7; t++)
            nbr_piece[c][t] = 0;

    for (int c=0; c<2; c++)
    {
        for (auto & p : pieces[c])
        {
            if (p.dead() == false)
                nbr_piece[c][p.type()]++;
        }
    }

    /* no more pawns */
    if(!nbr_piece[WHITE][PAWN] && !nbr_piece[BLACK][PAWN] )
    {
        /* nor heavies */
        if (!nbr_piece[WHITE][ROOK] && !nbr_piece[BLACK][ROOK] && !nbr_piece[WHITE][QUEEN] && !nbr_piece[BLACK][QUEEN])
        {
            if (!nbr_piece[BLACK][BISHOP] && !nbr_piece[WHITE][BISHOP])
            {
                /* only knights */
                /* it pretty safe to say this is a draw */
                if (nbr_piece[WHITE][KNIGHT] < 3 && nbr_piece[BLACK][KNIGHT] < 3)
                {
                    return true;
                }
            }
            else if (!nbr_piece[WHITE][KNIGHT] && !nbr_piece[BLACK][KNIGHT])
            {
                /* only bishops */
                /* not a draw if one side two other side zero
                          else its always a draw
             */
                if (abs(nbr_piece[WHITE][BISHOP] - nbr_piece[BLACK][BISHOP]) < 2)
                {
                    return true;
                }
            }
            else if ((nbr_piece[WHITE][KNIGHT] < 3 && !nbr_piece[WHITE][BISHOP]) || (nbr_piece[WHITE][BISHOP] == 1 && !nbr_piece[WHITE][KNIGHT]))
            {
                /* we cant win, but can black? */
                if ((nbr_piece[BLACK][KNIGHT] < 3 && !nbr_piece[BLACK][BISHOP]) || (nbr_piece[BLACK][BISHOP] == 1 && !nbr_piece[BLACK][KNIGHT]))
                {
                    /* guess not */
                    return true;
                }
            }
        }
        else if (!nbr_piece[WHITE][QUEEN] && !nbr_piece[BLACK][QUEEN])
        {
            if (nbr_piece[WHITE][ROOK] == 1 && nbr_piece[BLACK][ROOK] == 1)
            {
                /* rooks equal */
                if ((nbr_piece[WHITE][KNIGHT] + nbr_piece[WHITE][BISHOP]) < 2 && (nbr_piece[BLACK][KNIGHT] + nbr_piece[BLACK][BISHOP]) < 2)
                {
                    /* one minor difference max */
                    /* a draw too usually */
                    return true;
                }
            }
            else if (nbr_piece[WHITE][ROOK] == 1 && !nbr_piece[BLACK][ROOK])
            {
                /* one rook */
                /* draw if no minors to support AND
     minors to defend  */
                if ((nbr_piece[WHITE][KNIGHT] + nbr_piece[WHITE][BISHOP] == 0) && (((nbr_piece[BLACK][KNIGHT] + nbr_piece[BLACK][BISHOP]) == 1) || ((nbr_piece[BLACK][KNIGHT] + nbr_piece[BLACK][BISHOP]) == 2)))
                {
                    return true;
                }
            }
            else if (nbr_piece[BLACK][ROOK] == 1 && !nbr_piece[WHITE][ROOK])
            {
                /* one rook */
                /* draw if no minors to support AND
     minors to defend  */
                if ((nbr_piece[BLACK][KNIGHT] + nbr_piece[BLACK][BISHOP] == 0) && (((nbr_piece[WHITE][KNIGHT] + nbr_piece[WHITE][BISHOP]) == 1) || ((nbr_piece[WHITE][KNIGHT] + nbr_piece[WHITE][BISHOP]) == 2)))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

