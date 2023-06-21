#ifndef ORDERINGINFO_H
#define ORDERINGINFO_H

// Idée de Drofa

class OrderingInfo;

#include "defines.h"
#include "types.h"

//!
//! \brief Classe permettant d'ordonner les coups
//!  suivant divers heuristiques
//!

class OrderingInfo
{
public:
    OrderingInfo();
    void clearAllHistory();
    void clearKillers();


    MOVE getKiller1(int ply) const;
    MOVE getKiller2(int ply) const;
    int  getHistory(int color, PieceType piece, int dest) const;
    void incrementHistory(int color, PieceType piece, int to, int depth);
    void updateKillers(int ply, MOVE move);

private:

    /**
     * @brief Array of first killer moves by ply
     */
    U32 killer1[MAX_PLY];

    /**
     * @brief Array of second killer moves by ply
     */
    U32 killer2[MAX_PLY];

    /**
     * @brief Table of beta-cutoff history values indexed by [color][piece][to_square]
     */
    int history[2][7][64];


};

#endif // ORDERINGINFO_H
