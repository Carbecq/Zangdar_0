#include "defines.h"
#include "ChessBoard.h"

//-----------------------------------------------------
//! \brief Constructeur
//-----------------------------------------------------
ChessBoard::ChessBoard()
{
#ifdef DEBUG_CLASS
    std::cout << "ChessBoard::ChessBoard()" << std::endl;
#endif

}

//-----------------------------------------------------
//! \brief Destructeur
//-----------------------------------------------------
ChessBoard::~ChessBoard()
{
#ifdef DEBUG_CLASS
    std::cout << "ChessBoard::~ChessBoard()" << std::endl;
#endif

}

//-----------------------------------------------------
//! \brief Initialisation depuis la position de départ
//-----------------------------------------------------
void ChessBoard::init()
{
#ifdef DEBUG_CLASS
    std::cout << "ChessBoard::init()" << std::endl;
#endif

    search.init_fen(START_FEN);
}

//-----------------------------------------------------
//! \brief Initialisation depuis une chaine FEN
//-----------------------------------------------------
void ChessBoard::init(const std::string& fen)
{
#ifdef DEBUG_CLASS
    std::cout << "ChessBoard::init(fen)" << std::endl;
#endif

    search.init_fen(fen);
}

//-----------------------------------------------------
//! \brief Remise à zéro
//-----------------------------------------------------
void ChessBoard::reset()
{
#ifdef DEBUG_CLASS
    std::cout << "ChessBoard::reset" << std::endl;
#endif

    search.reset();
}



