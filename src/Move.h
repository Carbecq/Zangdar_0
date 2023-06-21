#ifndef LIBCHESS_MOVE_HPP
#define LIBCHESS_MOVE_HPP

#include <cassert>
#include <cstdint>
#include <ostream>
#include <sstream>
#include "defines.h"
#include "Square.h"


namespace Move
{

/* Codage des coups selon une idée de Weiss

0000 0000 0000 0000 0011 1111 -> From             <<  0
0000 0000 0000 1111 1100 0000 -> Dest             <<  6
0000 0000 0111 0000 0000 0000 -> Moving Piece     << 12
0000 0011 1000 0000 0000 0000 -> Captured Piece   << 15
0001 1100 0000 0000 0000 0000 -> Promotion Piece  << 18

0010 0000 0000 0000 0000 0000 -> Double
0100 0000 0000 0000 0000 0000 -> En-Passant
1000 0000 0000 0000 0000 0000 -> Castle

Note : la prise enpassant est aussi une capture

les bits 25 à 32 sont utilisés pour la Table de Transposition

0000 0111 0000 0000 0000 0000 0000 0000
          <--- move 24 bits        --->
      ccc hash_code 3 bits
uuuu u    inutilisés

il faut 3 bits pour stocker le hash code :
    HASH_NONE=0 (000), HASH_ALPHA=1 (001), HASH_BETA=2 (010), HASH_EXACT=4 (100)


*/

constexpr int     SHIFT_FROM  = 0;
constexpr int     SHIFT_DEST  = 6;
constexpr int     SHIFT_PIECE = 12;
constexpr int     SHIFT_CAPT  = 15;
constexpr int     SHIFT_PROMO = 18;
constexpr int     SHIFT_FLAGS = 21;
constexpr int     SHIFT_CODE  = 24;

constexpr U32 MOVE_NONE = 0;

// Fields
constexpr U32 MOVE_FROM      = 0b000000000000000000111111;
constexpr U32 MOVE_DEST      = 0b000000000000111111000000;
constexpr U32 MOVE_PIECE     = 0b000000000111000000000000;
constexpr U32 MOVE_CAPT      = 0b000000111000000000000000;
constexpr U32 MOVE_PROMO     = 0b000111000000000000000000;

constexpr U32 MOVE_FLAGS     = 0b111000000000000000000000;

// Special move flags
constexpr U32 FLAG_NONE      = 0;
constexpr U32 FLAG_DOUBLE    = 0b001000000000000000000000;
constexpr U32 FLAG_ENPASSANT = 0b010000000000000000000000;
constexpr U32 FLAG_CASTLE    = 0b100000000000000000000000;

constexpr U32 MOVE_DEPL      = 0b111111111000000000000000;  // mettre l'inverse ?

constexpr U32 MOVE_MOVE      = 0b000111111111111111111111111;


/* X Y  X&Y  X|Y  X^Y
 * 0 0  0    0    0
 * 0 1  0    1    1
 * 1 0  0    1    1
 * 1 1  1    1    0
 */



[[nodiscard]] constexpr U32 CODE(
    const int          _from,
    const int          _dest,
    const PieceType    _piece,
    const PieceType    _captured,
    const PieceType    _promotion,
    const U32          _flags)
{
    return( static_cast<U32>(_from)      << SHIFT_FROM  |
            static_cast<U32>(_dest)      << SHIFT_DEST  |
            static_cast<U32>(_piece)     << SHIFT_PIECE |
            static_cast<U32>(_captured)  << SHIFT_CAPT  |
            static_cast<U32>(_promotion) << SHIFT_PROMO |
            _flags
            );
}

//! \brief  Retourne la case de départ
[[nodiscard]] constexpr int from(const U32 move) noexcept {
    return (move & MOVE_FROM);
}

//! \brief  Retourne la case d'arrivée
[[nodiscard]] constexpr int dest(const U32 move) noexcept {
    return ((move & MOVE_DEST) >> SHIFT_DEST);
}

//! \brief  Retourne la pièce se déplaçant
[[nodiscard]] constexpr PieceType piece(const U32 move) noexcept {
    return static_cast<PieceType>((move & MOVE_PIECE) >> SHIFT_PIECE);
}

//! \brief  Retourne la pièce prise
[[nodiscard]] constexpr PieceType captured(const U32 move) noexcept {
    return static_cast<PieceType>((move & MOVE_CAPT) >> SHIFT_CAPT);
}

//! \brief  Retourne la pièce de promotion
[[nodiscard]] constexpr PieceType promotion(const U32 move)  noexcept {
    return static_cast<PieceType>(((move & MOVE_PROMO) >> SHIFT_PROMO));
}

//! \brief  Retourne les flags du coup
[[nodiscard]] constexpr U32 flags(const U32 move)  noexcept {
    return static_cast<PieceType>(move & MOVE_FLAGS);
}

//! \brief Détermine si le coup est un déplacement simple
[[nodiscard]] constexpr bool is_depl(const U32 move) noexcept {
    return ((move & MOVE_DEPL)==MOVE_NONE);
}

//! \brief Détermine si le coup est une capture, y compris capture avec promotion et prise enpassant
[[nodiscard]] constexpr bool is_capturing(const U32 move) noexcept {
    return (move & MOVE_CAPT);
}

//! \brief Détermine si le coup est une promotion
[[nodiscard]] constexpr bool is_promoting(const U32 move) noexcept {
    return (move & MOVE_PROMO);
}

//! \brief Détermine si le coup est une poussée double de pion
[[nodiscard]] constexpr bool is_double(const U32 move) noexcept {
    return (move & FLAG_DOUBLE);
}

//! \brief Détermine si le coup est un roque
[[nodiscard]] constexpr bool is_castling(const U32 move) noexcept {
    return (move & FLAG_CASTLE);
}

//! \brief Détermine si le coup est une prise en passant
[[nodiscard]] constexpr bool is_enpassant(const U32 move) noexcept {
    return (move & FLAG_ENPASSANT);
}

//! \brief Détermine si le coup est tactique : prise ou promotion ou prise en passant
[[nodiscard]] constexpr bool is_tactical(const U32 move) noexcept {
    return (move & (MOVE_CAPT | MOVE_PROMO | FLAG_ENPASSANT));
}

//---------------------------------------------------------------Pour la table de transposition

//! \brief  Retourne le hash_code
[[nodiscard]] constexpr int get_code(const U32 move) noexcept {
    return (move >>SHIFT_CODE);
}

//! \brief  Retourne le move
[[nodiscard]] constexpr MOVE get_move(const U32 move) noexcept {
    return (move & MOVE_MOVE);
}

//! \brief  Retourne le move + code
[[nodiscard]] constexpr MOVE get_code_move(const U32 move, const int code) noexcept {
    MOVE m = move & MOVE_MOVE;          // clearing;
    return (m | (code << SHIFT_CODE));
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
    if (Move::promotion(move) >= PieceType::Knight)
    {
        const char asd[] = {'?', 'p', 'n', 'b', 'r', 'q'};
        str += asd[Move::promotion(move)];
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



