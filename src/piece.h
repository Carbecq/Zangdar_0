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

//*******************************************************
//  Valeur des pièces
//  Values in centi-pawns of the pieces.
//  valeurs un peu différentes de Gerbil, à voir ...
//-------------------------------------------------------
enum PieceValue {
    valPAWN   =  100,
    valKNIGHT =  325,
    valBISHOP =  350,
    valROOK	  =  550,
    valQUEEN  = 1000,
    valKING   =    0	// il y a toujours un roi
};

static constexpr std::array<PieceValue, 6> PIECEVALUES = {
    valPAWN,
    valKNIGHT,
    valBISHOP,
    valROOK,
    valQUEEN,
    valKING
};

static const std::array<std::string, 6> nom_piece_max = { "", "N", "B", "R", "Q", "K"};
static const std::array<std::string, 6> nom_piece_min = { "", "n", "b", "r", "q", "k"};

#endif
