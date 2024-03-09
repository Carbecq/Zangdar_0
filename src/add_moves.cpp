#include "Board.h"
#include "Move.h"

//=================================================================
//! \brief  Ajoute un coup tranquille à la liste des coups
//!
//! \param[in]  type    type du coup
//! \param[in]  ml      MoveListde stockage des coups
//! \param[in]  from    position de départ de la pièce
//! \param[in]  dest    position d'arrivée de la pièce
//! \param[in]  piece   type de la pièce jouant
//-----------------------------------------------------------------
void Board::add_quiet_move(MoveList& ml, int from, int dest, PieceType piece, U32 flags)  const noexcept
{
    ml.moves[ml.count++] = Move::CODE(from, dest, piece, PieceType::NO_TYPE, PieceType::NO_TYPE, flags);
}

//=================================================================
//! \brief  Ajoute un coup de capture à la liste des coups
//!
//! \param[in]  type    type du coup
//! \param[in]  ml      MoveListde stockage des coups
//! \param[in]  from    position de départ de la pièce
//! \param[in]  dest    position d'arrivée de la pièce
//! \param[in]  piece   type de la pièce jouant
//! \param[in]  captured   type de la pièce capturée
//-----------------------------------------------------------------
void Board::add_capture_move(MoveList& ml, int from, int dest, PieceType piece, PieceType captured, U32 flags) const noexcept
{
    ml.moves[ml.count++]  = Move::CODE(from, dest, piece, captured, PieceType::NO_TYPE, flags);
}

//=================================================================
//! \brief  Ajoute un coup tranquille de promotion à la liste des coups
//!
//! \param[in]  type    type du coup
//! \param[in]  ml      MoveListde stockage des coups
//! \param[in]  from    position de départ de la pièce
//! \param[in]  dest    position d'arrivée de la pièce
//! \param[in]  promo   type de la pièce promue
//-----------------------------------------------------------------
void Board::add_quiet_promotion(MoveList& ml, int from, int dest, PieceType promo) const noexcept
{
    ml.moves[ml.count++] = Move::CODE(from, dest, PAWN, PieceType::NO_TYPE, promo, Move::FLAG_NONE);
}

//=================================================================
//! \brief  Ajoute un coup de capture et de promotion à la liste des coups
//!
//! \param[in]  type    type du coup
//! \param[in]  ml      MoveListde stockage des coups
//! \param[in]  from    position de départ de la pièce
//! \param[in]  dest    position d'arrivée de la pièce
//! \param[in]  piece   type de la pièce jouant
//! \param[in]  captured   type de la pièce capturée
//! \param[in]  promo   type de la pièce promue
//-----------------------------------------------------------------
void Board::add_capture_promotion(MoveList& ml, int from, int dest, PieceType captured, PieceType promo) const noexcept
{
    ml.moves[ml.count++]  = Move::CODE(from, dest, PAWN, captured, promo, Move::FLAG_NONE);
}




//===================================================================
//! \brief  Ajoute une série de coups
//-------------------------------------------------------------------
void Board::push_quiet_moves(MoveList& ml, Bitboard attack, const int from)
{
    int to;

    while (attack) {
        to = BB::pop_lsb(attack);
        add_quiet_move(ml, from, to, pieceOn[from], Move::FLAG_NONE);
    }
}
void Board::push_capture_moves(MoveList& ml, Bitboard attack, const int from)
{
    int to;

    while (attack) {
        to = BB::pop_lsb(attack);
        add_capture_move(ml, from, to, pieceOn[from], pieceOn[to], Move::FLAG_NONE);
    }
}

void Board::push_piece_quiet_moves(MoveList& ml, Bitboard attack, const int from, PieceType piece)
{
    int to;

    while (attack) {
        to = BB::pop_lsb(attack);
        add_quiet_move(ml, from, to, piece, Move::FLAG_NONE);
    }
}

void Board::push_piece_capture_moves(MoveList& ml, Bitboard attack, const int from, PieceType piece)
{
    int to;

    while (attack) {
        to = BB::pop_lsb(attack);
        add_capture_move(ml, from, to, piece, pieceOn[to], Move::FLAG_NONE);
    }
}

//--------------------------------------------------------------------
//  Promotions

void Board::push_quiet_promotions(MoveList& ml, Bitboard attack, const int dir) {
    int to;

    while (attack) {
        to = BB::pop_lsb(attack);
        push_quiet_promotion(ml, to - dir, to);
    }
}
void Board::push_capture_promotions(MoveList& ml, Bitboard attack, const int dir) {
    int to;

    while (attack) {
        to = BB::pop_lsb(attack);
        push_capture_promotion(ml, to - dir, to);
    }
}

/* Append promotions from the same move */
void Board::push_quiet_promotion(MoveList& ml, const int from, const int to) {
    add_quiet_promotion(ml, from, to, QUEEN);
    add_quiet_promotion(ml, from, to, KNIGHT);
    add_quiet_promotion(ml, from, to, ROOK);
    add_quiet_promotion(ml, from, to, BISHOP);
}
void Board::push_capture_promotion(MoveList& ml, const int from, const int to) {
    add_capture_promotion(ml, from, to, pieceOn[to], QUEEN);
    add_capture_promotion(ml, from, to, pieceOn[to], KNIGHT);
    add_capture_promotion(ml, from, to, pieceOn[to], ROOK);
    add_capture_promotion(ml, from, to, pieceOn[to], BISHOP);
}

//--------------------------------------
//  Coups de pions

void Board::push_pawn_quiet_moves(MoveList& ml, Bitboard attack, const int dir, U32 flags)
{
    int to;

    while (attack) {
        to = BB::pop_lsb(attack);
        add_quiet_move(ml, to - dir, to, PAWN, flags);
    }
}
void Board::push_pawn_capture_moves(MoveList& ml, Bitboard attack, const int dir)
{
    int to;

    while (attack) {
        to = BB::pop_lsb(attack);
        add_capture_move(ml, to - dir, to, PAWN, pieceOn[to], Move::FLAG_NONE);
    }
}




