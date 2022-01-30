#include <string>
#include <vector>
#include <algorithm>
#include <cassert>

#include "defines.h"
#include "Board.h"

//-----------------------------------------------------
//! \brief Affichage ascii de l'échiquier
//-----------------------------------------------------
void Board::display_ascii() const
{
    Square  sq;
    Piece*  p;
    std::string  s;
    char    c;

    std::vector<Rank> aa = RANKS;

    for (int i=0; i<9; i++)
        std::cout << "|---";
    std::cout << "|" << std::endl;

    reverse(aa.begin(), aa.end());
    for (Rank r : aa)
    {
        std::cout << "| " << r+1 << " | ";
        for (File f:FILES)
        {
            sq = square(r, f);
            p  = board[sq];
            assert(p != nullptr);
            s  = " ";

            if (p->type() != EMPTY)
            {
                s = p->to_string();
            }

            std::cout << s << " | " ;
        }
        std::cout << std::endl;

        for (int i=0; i<9; i++)
            std::cout << "|---";
        std::cout << "|" << std::endl;
    }

    std::cout << "|   ";

    for (int i=0; i<8; i++)
    {
        c = 0x41 + i;
        std::cout << "| " << c  << " ";
    }
    std::cout << "|" << std::endl;

    for (int i=0; i<9; i++)
        std::cout << "|---";
    std::cout << "|" << std::endl;


    std::cout << std::endl;

    if (positions->side_to_move == WHITE)
        std::cout << "side   : White" << std::endl;
    else
        std::cout << "side   : Black" << std::endl;
    std::cout << "enPas  : " << positions->ep_square << std::endl;
    std::cout << "castle : " <<
            (positions->castle & CASTLE_WK ? 'K' : '-') <<
            (positions->castle & CASTLE_WQ ? 'Q' : '-') <<
            (positions->castle & CASTLE_BK ? 'k' : '-') <<
            (positions->castle & CASTLE_BQ ? 'q' : '-') << std::endl;

    std::cout << "PosKey : " << positions->hash << std::endl;

}

