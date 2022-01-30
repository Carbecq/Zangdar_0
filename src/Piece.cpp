#include <cassert>
#include "Piece.h"

Piece::Piece()
{
    Piece(EMPTY, WHITE, OFFBOARD);
}

Piece::Piece(PieceType t, Color c, Square s)
{
    _type   = t;
    _color  = c;
    _square = s;
    _dead   = false;
}

std::ostream& operator<<(std::ostream& out, const Piece piece)
{
    return (out << piece.to_string());
}
std::string Piece::to_string() const
{
    char t;
    switch (type()) {
    case PAWN:
        t = 'I';
        break;
    case KNIGHT:
        t = 'N';
        break;
    case BISHOP:
        t = 'B';
        break;
    case ROOK:
        t = 'R';
        break;
    case QUEEN:
        t = 'Q';
        break;
    case KING:
        t = 'K';
        break;
    default:
        t = ' ';
        break;
    }
    if (color() == BLACK)
        t = static_cast<char>(t + 'a' - 'A'); // Lower case for black pieces

    return std::string(1, t);
}
char Piece::to_char() const
{
    char t;
    switch (type()) {
    case PAWN:
        t = 'P';
        break;
    case KNIGHT:
        t = 'N';
        break;
    case BISHOP:
        t = 'B';
        break;
    case ROOK:
        t = 'R';
        break;
    case QUEEN:
        t = 'Q';
        break;
    case KING:
        t = 'K';
        break;
    default:
        t = ' ';
        break;
    }
    if (color() == BLACK) {
        t = static_cast<char>(t + 'a' - 'A'); // Lower case for black pieces
    }
    return (t);
}

std::string Piece::to_name() const
{
    std::string t;
    switch (type()) {
    case PAWN:
        t = "pawn";
        break;
    case KNIGHT:
        t = "knight";
        break;
    case BISHOP:
        t = "bishop";
        break;
    case ROOK:
        t = "rook";
        break;
    case QUEEN:
        t = "queen";
        break;
    case KING:
        t = "king";
        break;
    default:
        t = "";
        break;
    }
    if (color() == BLACK)
        t = "b_" + t;
    else
        t = "w_" + t;

    return (t);
}

