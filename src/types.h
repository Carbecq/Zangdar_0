#ifndef TYPES_H
#define TYPES_H

#include <array>
#include <string>
#include "defines.h"

/*******************************************************
 ** Les couleurs
 **---------------------------------------------------*/

#include <string>

constexpr int N_COLORS = 2;

enum Color : int
{
    WHITE    = 0,
    BLACK    = 1
};

const std::string side_name[N_COLORS] = {"White", "Black"};
const std::string camp[2][N_COLORS] = {
    {"Blanche", "Noire"}, {"Blanc", "Noir"}
};

//Inverts the color (WHITE -> BLACK) and (BLACK -> WHITE)
constexpr Color operator~(Color C) { return Color(C ^ BLACK); }

/*******************************************************
 ** Les pièces
 **---------------------------------------------------*/

constexpr int N_PIECES = 7;

enum PieceType : int
{
    NO_TYPE  = 0,
    PAWN     = 1,
    KNIGHT   = 2,
    BISHOP   = 3,
    ROOK     = 4,
    QUEEN    = 5,
    KING     = 6
};

const std::string piece_name[N_PIECES] {
    "NONE", "Pawn", "Knight", "Bishop", "Rook", "Queen", "King"
};

const std::string piece_symbol[N_COLORS][N_PIECES] {
                                     {  "?", "P", "N", "B", "R", "Q", "K"},
                                     {  "?", "p", "n", "b", "r", "q", "k"},
                                     };

static const std::array<std::string, N_PIECES> nom_piece_max = { "?", "", "N", "B", "R", "Q", "K"};
static const std::array<std::string, N_PIECES> nom_piece_min = { "?", "", "n", "b", "r", "q", "k"};


/*******************************************************
 ** Les cases
 **---------------------------------------------------*/

constexpr int N_SQUARES = 64;

enum EnumSquare : int {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NO_SQUARE
};

const std::string square_name[N_SQUARES] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};

enum Rank : int { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, NO_RANK };
enum File : int { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, NO_FILE };

constexpr int N_RANKS = 8;
constexpr int N_FILES = 8;

// https://www.chessprogramming.org/General_Setwise_Operations#Shifting_Bitboards

enum Direction : int {
    NORTH       = 8,
    NORTH_EAST  = 9,
    EAST        = 1,
    SOUTH_EAST  = -7,
    SOUTH       = -8,
    SOUTH_WEST  = -9,
    WEST        = -1,
    NORTH_WEST  = 7
} ;

struct PVariation {
    MOVE line[MAX_PLY+1];
    int  length;
};

#endif // TYPES_H

