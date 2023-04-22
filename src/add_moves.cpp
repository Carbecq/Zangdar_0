#include "Board.h"

//=================================================================
//! \brief  Ajoute un coup tranquille à la liste des coups
//!
//! \param[in]  type    type du coup
//! \param[in]  ml      MoveListde stockage des coups
//! \param[in]  from    position de départ de la pièce
//! \param[in]  dest    position d'arrivée de la pièce
//! \param[in]  piece   type de la pièce jouant
//-----------------------------------------------------------------
void Board::add_quiet_move(MoveType type, MoveList& ml, int from, int dest, PieceType piece)  const noexcept
{
    ml.moves[ml.count++] = Move::CODE(type, from, dest, piece, PieceType::NO_TYPE, PieceType::NO_TYPE);
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
void Board::add_capture_move(MoveType type, MoveList& ml, int from, int dest, PieceType piece, PieceType captured) const noexcept
{
    ml.moves[ml.count++]  = Move::CODE(type, from, dest, piece, captured, PieceType::NO_TYPE);
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
void Board::add_quiet_promotion(MoveType type, MoveList& ml, int from, int dest, PieceType promo) const noexcept
{
    ml.moves[ml.count++] = Move::CODE(type, from, dest, PieceType::Pawn, PieceType::NO_TYPE, promo);
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
void Board::add_capture_promotion(MoveType type, MoveList& ml, int from, int dest, PieceType captured, PieceType promo) const noexcept
{
    ml.moves[ml.count++]  = Move::CODE(type, from, dest, PieceType::Pawn, captured, promo);
}




//===================================================================
//! \brief  Ajoute une série de coups
//-------------------------------------------------------------------
void Board::push_quiet_moves(MoveList& ml, Bitboard attack, const int from)
{
    int to;

    while (attack) {
        to = next_square(attack);
        add_quiet_move(MoveType::Normal, ml, from, to, cpiece[from]);
    }
}
void Board::push_capture_moves(MoveList& ml, Bitboard attack, const int from)
{
    int to;

    while (attack) {
        to = next_square(attack);
        add_capture_move(MoveType::Capture, ml, from, to, cpiece[from], cpiece[to]);
    }
}

void Board::push_piece_quiet_moves(MoveList& ml, Bitboard attack, const int from, PieceType piece)
{
    int to;

    while (attack) {
        to = next_square(attack);
        add_quiet_move(MoveType::Normal, ml, from, to, piece);
    }
}

void Board::push_piece_capture_moves(MoveList& ml, Bitboard attack, const int from, PieceType piece)
{
    int to;

    while (attack) {
        to = next_square(attack);
        add_capture_move(MoveType::Capture, ml, from, to, piece, cpiece[to]);
    }
}

//--------------------------------------------------------------------
//  Promotions

void Board::push_quiet_promotions(MoveList& ml, Bitboard attack, const int dir) {
    int to;

    while (attack) {
        to = next_square(attack);
        push_quiet_promotion(ml, to - dir, to);
    }
}
void Board::push_capture_promotions(MoveList& ml, Bitboard attack, const int dir) {
    int to;

    while (attack) {
        to = next_square(attack);
        push_capture_promotion(ml, to - dir, to);
    }
}

/* Append promotions from the same move */
void Board::push_quiet_promotion(MoveList& ml, const int from, const int to) {
    add_quiet_promotion(MoveType::Promotion, ml, from, to, PieceType::Queen);
    add_quiet_promotion(MoveType::Promotion, ml, from, to, PieceType::Knight);
    add_quiet_promotion(MoveType::Promotion, ml, from, to, PieceType::Rook);
    add_quiet_promotion(MoveType::Promotion, ml, from, to, PieceType::Bishop);
}
void Board::push_capture_promotion(MoveList& ml, const int from, const int to) {
    add_capture_promotion(MoveType::PromotionCapture, ml, from, to, cpiece[to], PieceType::Queen);
    add_capture_promotion(MoveType::PromotionCapture, ml, from, to, cpiece[to], PieceType::Knight);
    add_capture_promotion(MoveType::PromotionCapture, ml, from, to, cpiece[to], PieceType::Rook);
    add_capture_promotion(MoveType::PromotionCapture, ml, from, to, cpiece[to], PieceType::Bishop);
}

//--------------------------------------
//  Coups de pions

void Board::push_pawn_quiet_moves(MoveType type, MoveList& ml, Bitboard attack, const int dir) {
    int to;

    while (attack) {
        to = next_square(attack);
        add_quiet_move(type, ml, to - dir, to, PieceType::Pawn);
    }
}
void Board::push_pawn_capture_moves(MoveList& ml, Bitboard attack, const int dir) {
    int to;

    while (attack) {
        to = next_square(attack);
        add_capture_move(MoveType::Capture, ml, to - dir, to, PieceType::Pawn, cpiece[to]);
    }
}




