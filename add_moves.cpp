#include "Board.h"

//=================================================================
//! \brief  Ajout d'un coup tranquille à la liste des coups
//!
//! \param[in]  from    position de départ de la pièce
//! \param[in]  to      position d'arrivée de la pièce
//! \param[in]  flags   flags déterminant le type du coup
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::add_quiet_move(MoveType type, MoveList& ml, int from, int dest, PieceType piece)  const noexcept
{
    ml.moves[ml.count++] = Move::CODE(type, from, dest, piece, PieceType::NO_TYPE, PieceType::NO_TYPE);

}

//=================================================================
//! \brief  Ajout d'un coup de capture à la liste des coups
//!
//! \param[in]  from    position de départ de la pièce
//! \param[in]  to      position d'arrivée de la pièce
//! \param[in]  flags   flags déterminant le type du coup
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::add_capture_move(MoveType type, MoveList& ml, int from, int dest, PieceType piece, PieceType captured) const noexcept
{
    ml.moves[ml.count++]  = Move::CODE(type, from, dest, piece, captured, PieceType::NO_TYPE);
}

//=================================================================
//! \brief  Ajout d'un coup tranquille à la liste des coups
//!
//! \param[in]  from    position de départ de la pièce
//! \param[in]  to      position d'arrivée de la pièce
//! \param[in]  flags   flags déterminant le type du coup
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::add_quiet_promotion(MoveType type, MoveList& ml, int from, int dest, PieceType promo) const noexcept
{
    ml.moves[ml.count++] = Move::CODE(type, from, dest, PieceType::Pawn, PieceType::NO_TYPE, promo);
}

//=================================================================
//! \brief  Ajout d'un coup tranquille à la liste des coups
//!
//! \param[in]  from    position de départ de la pièce
//! \param[in]  to      position d'arrivée de la pièce
//! \param[in]  flags   flags déterminant le type du coup
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::add_capture_promotion(MoveType type, MoveList& ml, int from, int dest, PieceType captured, PieceType promo) const noexcept
{
    ml.moves[ml.count++]  = Move::CODE(type, from, dest, PieceType::Pawn, captured, promo);
}




/* Append a move to an array of moves */

/* Append all moves from a square */
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
/* Append all promotions from a direction */
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
    add_quiet_promotion(MoveType::promo, ml, from, to, PieceType::Queen);
    add_quiet_promotion(MoveType::promo, ml, from, to, PieceType::Knight);
    add_quiet_promotion(MoveType::promo, ml, from, to, PieceType::Rook);
    add_quiet_promotion(MoveType::promo, ml, from, to, PieceType::Bishop);
}
void Board::push_capture_promotion(MoveList& ml, const int from, const int to) {
    add_capture_promotion(MoveType::promo_capture, ml, from, to, cpiece[to], PieceType::Queen);
    add_capture_promotion(MoveType::promo_capture, ml, from, to, cpiece[to], PieceType::Knight);
    add_capture_promotion(MoveType::promo_capture, ml, from, to, cpiece[to], PieceType::Rook);
    add_capture_promotion(MoveType::promo_capture, ml, from, to, cpiece[to], PieceType::Bishop);
}
//--------------------------------------
/* Append all pawn moves from a direction */
void Board::push_pawn_quiet_moves(MoveType type, MoveList& ml, Bitboard attack, const int dir) {
    int to;

    while (attack) {
        to = next_square(attack);
        add_quiet_move(type, ml, to - dir, to, PieceType::Pawn);
    }
}
/* Append all pawn moves from a direction */
void Board::push_pawn_capture_moves(MoveList& ml, Bitboard attack, const int dir) {
    int to;

    while (attack) {
        to = next_square(attack);
        add_capture_move(MoveType::Capture, ml, to - dir, to, PieceType::Pawn, cpiece[to]);
    }
}



