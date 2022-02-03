#ifndef POSITION_H
#define POSITION_H

class Position;

#include <array>
#include "defines.h"
#include "Piece.h"

// structure destinée à stocker l'historique de make_move.
// celle-ci sera nécessaire pour effectuer un unmake_move

typedef struct {
    Square  ep_square;          // case en-passant : si les blancs jouent e2-e4, la case est e3
    Square  ep_took;            //                                                           e4
    U32     castle;             // droit au roque

    int     fifty;              // nombre de coups depuis une capture, ou un movement de pion
    U64     hash;               // nombre unique (?) correspondant à la position

    Piece*  captured;           // pointeur sur la pièce capturée, dans la liste des pièces
} History;

class Position
{
public:
    Position();

    void init(Color c);
    void reset();

    Color   side_to_move;       // camp au trait
    Square  ep_square;          // case en-passant : si les blancs jouent e2-e4, la case est e3
    Square  ep_took;            //                                                           e4
    U32     castle;             // droit au roque
    int     fifty;              // nombre de coups depuis une capture, ou un movement de pion
    U64     hash;               // nombre unique (?) correspondant à la position (clef Zobrist)

    std::vector<std::string>  best;       // meilleur coup (pour les test perft)

    int     ply;                // profondeur de recherche, en demi-coups
    int     hply;               // nombre de demi-coups de la partie

    int  is_repetition(const std::array<History, MAX_HIST>& history) const;
    bool is_draw(const std::array<History, MAX_HIST>& history) const;

private:

};

#endif // POSITION_H

