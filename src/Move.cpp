
#include <string>
#include <iostream>
#include <sstream>
#include <cassert>

#include "Move.h"
#include "Piece.h"


Move::Move()
{
    _code  = 0;
    _score = 0;
}
Move::Move(Square from, Square dest, PieceType type, U32 flags)
{
    setCode(from, dest, type, EMPTY, flags);
    _score  = 0;
}


//========================================
//! \brief  Affichage d'un coup
//! \param[in]  mode    détermine la manière d'écrire le coup
//!
//! mode = 0 : Ff1-c4
//!        1 : Fc4      : résultat Win At Chess
//!        2 : Nbd7
//!        3 : R1a7
//!        4 : f1c3     : communication UCI
//!
//----------------------------------------
std::string Move::show(int mode) const
{
    // voir Purple Haze : output.cpp et protocol.cpp et main.cpp

    std::stringstream ss;
    std::string s;

    if ((castle_wk() || castle_bk() || castle_wq() || castle_bq() ) && (mode != 4))
    {
            if (castle_wk() || castle_bk())
                ss << "0-0";
            else if (castle_wq() || castle_bq())
                ss << "0-0-0";
    }
    else
    {
        File file_from = file(from());
        File file_to   = file(dest());

        Rank rank_from = rank(from());
        Rank rank_to   = rank(dest());

        std::string nom_piece_max[] = { "VIDE", "", "N", "B", "R", "Q", "K"};
        std::string nom_piece_min[] = { "VIDE", "", "n", "b", "r", "q", "k"};

        char cfile_from = 'a' + file_from;
        char crank_from = '1' + rank_from;

        char cfile_to = 'a' + file_to;
        char crank_to = '1' + rank_to;

        PieceType ptype = type();

        if (mode != 4)
        {
            ss << nom_piece_max[ptype];
        }

        if (ptype != PAWN)
        {
            if (mode == 2)
                ss << cfile_from;
            else if (mode == 3)
                ss << crank_from;
        }

        if (mode == 0 || mode == 4)
        {
            ss << cfile_from;
            ss << crank_from;
        }
        else if (mode == 1)
        {
            if (ptype == PAWN && capture())
                ss << cfile_from;
        }

        if (mode !=4 && capture())
            ss << "x";
        else if (mode == 0)
            ss << "-";

        ss << cfile_to;
        ss << crank_to;

        if (promotion())
        {
            if (mode == 4)
                ss << nom_piece_min[promotion()];
            else
                ss << ":" << nom_piece_max[promotion()];
        }
    }

    ss >> s;

    return(s);
}
//===========================================================
//! \brief  teste l'égalité du coup avec celui donné par une chaine de caractère
//! \param[in]  str     coup à tester
//!     str = Bc4
//!     str = Nbd7
//!     str = R1a7
//!
//! \return true si égalité
//-----------------------------------------------------------
bool Move::equal(const std::string & str)
{
    if (show(1)==str || show(2)==str || show(3)==str)
        return(true);
    else
        return(false);
}
