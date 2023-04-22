#ifndef LIBCHESS_PIECE_HPP
#define LIBCHESS_PIECE_HPP

#include <array>
#include <string>

enum PieceType : int
{
    Pawn = 0,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    NO_TYPE,
};

static constexpr std::array<PieceType, 7> AllPieceType = {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    NO_TYPE
};

const std::string piece_name[7] {
    "Pawn", "Knight", "Bishop", "Rook", "Queen", "King", "NONE"
};

const std::string piece_symbol[2][7] {
  {  "P", "N", "B", "R", "Q", "K", "?"},
  {  "p", "n", "b", "r", "q", "k", "?"},

};

//*******************************************************
//  Valeur des pièces
//  Values in centi-pawns of the pieces.
//  valeurs un peu différentes de Gerbil, à voir ...
//-------------------------------------------------------
enum PIECE_VALUE:int {
    PAWN_VALUE   =  100,
    KNIGHT_VALUE =  325,
    BISHOP_VALUE =  350,
    ROOK_VALUE	 =  550,
    QUEEN_VALUE  = 1000,
    KING_VALUE   =    0	// il y a toujours un roi
};

static constexpr std::array<PIECE_VALUE, 6> Piece_Value = {
    PAWN_VALUE,
    KNIGHT_VALUE,
    BISHOP_VALUE,
    ROOK_VALUE,
    QUEEN_VALUE,
    KING_VALUE
};

static const std::array<std::string, 6> nom_piece_max = { "", "N", "B", "R", "Q", "K"};
static const std::array<std::string, 6> nom_piece_min = { "", "n", "b", "r", "q", "k"};

#endif
