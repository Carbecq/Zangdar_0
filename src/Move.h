#ifndef LIBCHESS_MOVE_HPP
#define LIBCHESS_MOVE_HPP

#include <cassert>
#include <cstdint>
#include <ostream>
#include <sstream>
#include "piece.h"
#include "defines.h"
#include "Square.h"

enum MoveShifts
{
    SHIFT_FROM            = 0,
    SHIFT_DEST            = 6,
    SHIFT_TYPE            = 12,
    SHIFT_MOVING_PIECE    = 15,
    SHIFT_CAPTURED_PIECE  = 18,
    SHIFT_PROMOTION_PIECE = 21,
    SHIFT_SCORE_INFO      = 24,
};
enum MoveType : int
{
    Normal           = 0,
    Capture          = 1,
    Double           = 2,
    EnPassant        = 3,
    KingCastle       = 4,
    QueenCastle      = 5,
    Promotion        = 6,
    PromotionCapture = 7,    //  111
};

namespace Move
{
[[nodiscard]] constexpr U32 CODE(
                                 const MoveType     _type,
                                 const int          _from,
                                 const int          _dest,
                                 const PieceType    _piece,
                                 const PieceType    _captured,
                                 const PieceType    _promotion)
{
    return( static_cast<U32>(_from)      << SHIFT_FROM           |
            static_cast<U32>(_dest)      << SHIFT_DEST           |
            static_cast<U32>(_type)      << SHIFT_TYPE           |
            static_cast<U32>(_piece)     << SHIFT_MOVING_PIECE   |
            static_cast<U32>(_captured)  << SHIFT_CAPTURED_PIECE |
            static_cast<U32>(_promotion) << SHIFT_PROMOTION_PIECE );
}

/*  Int packing:
     *  +6  6 - From
     *  +6 12 - To
     *  +3 15 - Type
     *  +3 18 - Piece
     *  +3 21 - Captured Piece
     *  +3 24 - Promotion Piece
     *
     *  +8 32 - score (age)
     */

[[nodiscard]] constexpr int from(U32 move) noexcept {
    return (move & 0x3F);
}

[[nodiscard]] constexpr int dest(U32 move) noexcept {
    return ((move >> SHIFT_DEST) & 0x3F);
}

[[nodiscard]] constexpr MoveType type(U32 move) noexcept {          // type du coup
    return MoveType((move >> SHIFT_TYPE) & 0x7);
}

[[nodiscard]] constexpr PieceType piece(U32 move) noexcept {        // pièce jouée
    return static_cast<PieceType>((move >> SHIFT_MOVING_PIECE) & 0x7   );
}

[[nodiscard]] constexpr PieceType captured(U32 move) noexcept {     // pièce capturée
    return static_cast<PieceType>((move >> SHIFT_CAPTURED_PIECE) & 0x7);
}

[[nodiscard]] constexpr PieceType promotion(U32 move)  noexcept {   // pièce promue
    return static_cast<PieceType>((move >> SHIFT_PROMOTION_PIECE) & 0x7 );
}

// le score sert à stocker l'age (voir Koivisto)
template<uint8_t N>
constexpr uint32_t MASK = (1 << N) - 1;

/*
MASK<8>                          00000000000000000000000011111111
(MASK<8> << SHIFT_SCORE_INFO)    11111111000000000000000000000000
~(MASK<8> << SHIFT_SCORE_INFO)   00000000111111111111111111111111
*/

constexpr U32 set_flag(const U32 _move, const int _flag) noexcept {
    U32 move  = (_move & ~(MASK<8> << SHIFT_SCORE_INFO));    // clearing
        move |= (_flag << SHIFT_SCORE_INFO);
    return(move);
}
[[nodiscard]] constexpr int flag(const U32 _move) {
    return (_move >> SHIFT_SCORE_INFO);
}
[[nodiscard]] constexpr U32 move(const U32 _move) {
    return (_move & ~(MASK<8> << SHIFT_SCORE_INFO));
}

//! \brief Détermine si le coup est une capture
[[nodiscard]] constexpr bool is_capturing(U32 move) noexcept {
    return (type(move) == MoveType::Capture || type(move) == MoveType::PromotionCapture || type(move) == MoveType::EnPassant);
}

//! \brief Détermine si le coup est une promotion
[[nodiscard]] constexpr bool is_promoting(U32 move) noexcept {
    return type(move) == MoveType::Promotion || type(move) == MoveType::PromotionCapture;
}

//! \brief Détermine si le coup est un roque
[[nodiscard]] constexpr bool is_castling(U32 move) noexcept {
    return type(move) == MoveType::KingCastle || type(move) == MoveType::QueenCastle;
}

//! \brief Détermine si le coup est une prise en passanr
[[nodiscard]] constexpr bool is_enpassant(U32 move) noexcept {
    return type(move) == MoveType::EnPassant;
}


//=================================================
//! \brief Affichage du coup
//-------------------------------------------------
// exemples :
//  std::string s = Move::name(move) ;
//  std::cout << Move::name(move) ;
//-------------------------------------------------
[[nodiscard]] inline std::string name(U32 move) noexcept
{
    std::string str;
    str += square_name[move & 0x3F];
    str += square_name[(move >> 6) & 0x3F];
    if (Move::promotion(move) != PieceType::NO_TYPE)
    {
        const char asd[] = {'n', 'b', 'r', 'q'};
        str += asd[Move::promotion(move) - 1];
    }
    return str;
}

//========================================
//! \brief  Affichage d'un coup
//! \param[in]  mode    détermine la manière d'écrire le coup
//!
//! mode = 0 : Ff1-c4
//!        1 : Fc4      : résultat Win At Chess
//!        2 : Nbd7     : résultat Win At Chess
//!        3 : R1a7     : résultat Win At Chess
//!        4 : f1c3     : communication UCI
//!
//----------------------------------------
[[nodiscard]] inline std::string show(MOVE move, int mode) noexcept
{
    // voir Purple Haze : output.cpp et protocol.cpp et main.cpp

    std::stringstream ss;
    std::string s;

// ZZ   if ((CASTLE_WK(move) || CASTLE_BK(move) || CASTLE_WQ(move) || CASTLE_BQ(move) ) && (mode != 4))
//    {
//            if (CASTLE_WK(move) || CASTLE_BK(move))
//                ss << "0-0";
//            else if (CASTLE_WQ(move) || CASTLE_BQ(move))
//                ss << "0-0-0";
//    }
//    else
    {
        int file_from = Square::file(Move::from(move));
        int file_to   = Square::file(Move::dest(move));

        int rank_from = Square::rank(Move::from(move));
        int rank_to   = Square::rank(Move::dest(move));

        char cfile_from = 'a' + file_from;
        char crank_from = '1' + rank_from;

        char cfile_to = 'a' + file_to;
        char crank_to = '1' + rank_to;

        PieceType ptype = Move::piece(move);

        if (mode != 4)
        {
            ss << nom_piece_max[ptype];
        }

        if (ptype != PieceType::Pawn)
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
            if (ptype == PieceType::Pawn && Move::is_capturing(move))
                ss << cfile_from;
        }

        if (mode !=4 && Move::is_capturing(move))
            ss << "x";
        else if (mode == 0)
            ss << "-";

        ss << cfile_to;
        ss << crank_to;

        if (Move::is_promoting(move))
        {
            if (mode == 4)
                ss << nom_piece_min[Move::promotion(move)];
            else
                ss << "=" << nom_piece_max[Move::promotion(move)];
        }
    }

    ss >> s;

    return(s);
}

inline std::ostream &operator<<(std::ostream &os, const U32 move) noexcept
{
    os << name(move);
    return os;
}

}

#endif



