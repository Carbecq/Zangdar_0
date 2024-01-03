#include <sstream>
#include "Board.h"

//===================================================
//! \brief  Constructeur
//---------------------------------------------------
Board::Board() noexcept
{
    init_bitmasks();
}

Board::Board(const std::string &fen) noexcept
{
    init_bitmasks();
    set_fen(fen, false);
}

//===================================================
//! \brief  Affichage de l'échiquier
//---------------------------------------------------
std::string Board::display() const noexcept
{
    std::stringstream ss;

    int i = 56;
    U64 hh = get_hash();
    U64 ph = get_pawn_hash();
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
    ss <<     "Hash     : " << hh << "\n";
    ss <<     "PawnHash : " << ph << "\n";
    ss <<     "Fen      : " << get_fen();
    return(ss.str());
}

void Board::clear() noexcept
{
    colorPiecesBB[0] = 0ULL;
    colorPiecesBB[1] = 0ULL;

    typePiecesBB[0] = 0ULL;
    typePiecesBB[1] = 0ULL;
    typePiecesBB[2] = 0ULL;
    typePiecesBB[3] = 0ULL;
    typePiecesBB[4] = 0ULL;
    typePiecesBB[5] = 0ULL;
    typePiecesBB[6] = 0ULL;

    cpiece.fill(PieceType::NO_TYPE);

    x_king[WHITE] = NO_SQUARE;                      // position des rois
    x_king[BLACK] = NO_SQUARE;                      // position des rois

    halfmove_counter = 0;
    fullmove_counter = 1;
    gamemove_counter = 0;
    hash             = 0;
    pawn_hash        = 0;

    ep_square    = NO_SQUARE;
    castling     = CASTLE_NONE;
    side_to_move = Color::WHITE;
    
    game_history.fill(UndoInfo{0, 0, 0, 0, 0, 0});
}

template bool Board::is_in_check<WHITE>()     const noexcept;
template bool Board::is_in_check<BLACK>()     const noexcept;
