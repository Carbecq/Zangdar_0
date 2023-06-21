#ifndef TYPES_H
#define TYPES_H

#include <array>
#include <string>

/*******************************************************
 ** Les couleurs
 **---------------------------------------------------*/

#include <string>
enum Color : int
{
    WHITE = 0,
    BLACK
};

const std::string side_name[] = {"White", "Black"};

//Inverts the color (WHITE -> BLACK) and (BLACK -> WHITE)
constexpr Color operator~(Color C) { return Color(C ^ BLACK); }

/*******************************************************
 ** Les pièces
 **---------------------------------------------------*/

enum PieceType : int
{
    NO_TYPE = 0,
    Pawn    = 1,
    Knight  = 2,
    Bishop  = 3,
    Rook    = 4,
    Queen   = 5,
    King    = 6,
};

static constexpr std::array<PieceType, 7> AllPieceType = {
    NO_TYPE,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

const std::string piece_name[7] {
    "NONE", "Pawn", "Knight", "Bishop", "Rook", "Queen", "King"
};

const std::string piece_symbol[2][7] {
                                     {  "?", "P", "N", "B", "R", "Q", "K"},
                                     {  "?", "p", "n", "b", "r", "q", "k"},

                                     };

static const std::array<std::string, 7> nom_piece_max = { "?", "", "N", "B", "R", "Q", "K"};
static const std::array<std::string, 7> nom_piece_min = { "?", "", "n", "b", "r", "q", "k"};

enum PieceValue {
    P_MG =  101, P_EG =  160,
    N_MG =  399, N_EG =  486,
    B_MG =  408, B_EG =  499,
    R_MG =  567, R_EG =  875,
    Q_MG = 1386, Q_EG = 1588
};

/*******************************************************
 ** Les cases
 **---------------------------------------------------*/

enum Squares : int {
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

const std::string square_name[] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};

enum rank : int { RANK_1 = 0, RANK_2 = 1, RANK_3 = 2, RANK_4 = 3, RANK_5 = 4, RANK_6 = 5, RANK_7 = 6, RANK_8 = 7, NO_RANK = 8 };
enum file : int { FILE_A = 0, FILE_B = 1, FILE_C = 2, FILE_D = 3, FILE_E = 4, FILE_F = 5, FILE_G = 6, FILE_H = 7, NO_FILE = 8 };

// https://www.chessprogramming.org/General_Setwise_Operations#Shifting_Bitboards

typedef enum Direction {
    NORTH       = 8,
    NORTH_EAST  = 9,
    EAST        = 1,
    SOUTH_EAST  = -7,
    SOUTH       = -8,
    SOUTH_WEST  = -9,
    WEST        = -1,
    NORTH_WEST  = 7
} Direction;


#endif // TYPES_H
