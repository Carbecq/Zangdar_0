#include "MoveList.h"
#include "Move.h"

//============================================
//! \brief  Constructeur
//--------------------------------------------
MoveList::MoveList()
{
    count = 0;
}

//=============================================
//! \brief  échange 2 éléments
//---------------------------------------------
void MoveList::swap(size_t i, size_t j)
{
    std::swap(moves[i],  moves[j]);
    std::swap(values[i], values[j]);
}



