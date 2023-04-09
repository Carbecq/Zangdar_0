#include <sstream>
#include "Board.h"
#include "bitmask.h"

Bitboard RankMask64[64] = {0};
Bitboard FileMask64[64] = {0};
Bitboard DiagonalMask64[64] = {0};
Bitboard AntiDiagonalMask64[64] = {0};
Bitboard SquareMask64[64] = {0};


//===================================================
//! \brief  Constructeur
//---------------------------------------------------
Board::Board() noexcept
{
    init_allmask();
}

Board::Board(const std::string &fen) noexcept
{
    init_allmask();
    set_fen(fen, false);
}


//===================================================
//! \brief  Remise à zéro
//---------------------------------------------------
void Board::reset()
{

}

//===================================================
//! \brief  Affichage de l'échiquier
//---------------------------------------------------
std::string Board::display() const noexcept
{
    std::stringstream ss;

    int i = 56;
    U64 hh = get_hash();
    ss << std::endl;

    ss << "  8 " ;

    while (i >= 0) {
        const auto sq = i;
        const auto bb = square_to_bit(sq);

        if (pieces_cp<Color::WHITE, PieceType::Pawn>() & bb) {
            ss << 'P';
        } else if (pieces_cp<Color::WHITE, PieceType::Knight>() & bb) {
            ss << 'N';
        } else if (pieces_cp<Color::WHITE, PieceType::Bishop>() & bb) {
            ss << 'B';
        } else if (pieces_cp<Color::WHITE, PieceType::Rook>() & bb) {
            ss << 'R';
        } else if (pieces_cp<Color::WHITE, PieceType::Queen>() & bb) {
            ss << 'Q';
        } else if (pieces_cp<Color::WHITE, PieceType::King>() & bb) {
            ss << 'K';
        } else if (pieces_cp<Color::BLACK, PieceType::Pawn>() & bb) {
            ss << 'p';
        } else if (pieces_cp<Color::BLACK, PieceType::Knight>() & bb) {
            ss << 'n';
        } else if (pieces_cp<Color::BLACK, PieceType::Bishop>() & bb) {
            ss << 'b';
        } else if (pieces_cp<Color::BLACK, PieceType::Rook>() & bb) {
            ss << 'r';
        } else if (pieces_cp<Color::BLACK, PieceType::Queen>() & bb) {
            ss << 'q';
        } else if (pieces_cp<Color::BLACK, PieceType::King>() & bb) {
            ss << 'k';
        } else {
            ss << '.';
        }
        ss << ' ';

        if (i % 8 == 7)
        {
            if (i/8 != 0)
                ss << "\n  " << i/8 << ' ';
            i -= 16;
        }

        i++;
    }
    ss << "\n    a b c d e f g h\n\n";

    ss << "Castling : ";
    ss << (white_can_castle_k() ? "K" : "");
    ss << (white_can_castle_q() ? "Q" : "");
    ss << (black_can_castle_k() ? "k" : "");
    ss << (black_can_castle_q() ? "q" : "");
    ss << '\n';
    if (ep() == NO_SQUARE) {
        ss << "EP       : -\n";
    } else {
        ss << "EP       : " << square_name[ep()] << '\n';
    }
    ss <<     "Turn     : " << (turn() == Color::WHITE ? 'w' : 'b') << '\n';
    ss <<     "Hash     : " << hh;

    return(ss.str());
}

//===========================================================================
// Regarde dans la liste des coups si le coup de la Principal Variation
// existe. Dans ce cas, on donne à ce coup un gros bonus de façon à ce
// qu'il soit joué en premier par la fonction de recherche.
//----------------------------------------------------------------------------
void Board::pv_move(MoveList& move_list, U32 PvMove)
{
    // https://www.chessprogramming.org/Principal_Variation

    for (int index = 0; index < move_list.count; index++)
    {
        if( move_list.moves[index] == PvMove)
        {
            move_list.values[index] = ( 2000000 );
            break;
        }
    }
}

template bool Board::is_in_check<WHITE>()     const noexcept;
template bool Board::is_draw<WHITE>() const noexcept;

template bool Board::is_in_check<BLACK>()     const noexcept;
template bool Board::is_draw<BLACK>() const noexcept;
