#include "Board.h"


[[nodiscard]] std::string fen_pieces(const Board &pos) noexcept {
    const char piece_chars[2][6] = {
        {'P', 'N', 'B', 'R', 'Q', 'K'},
        {'p', 'n', 'b', 'r', 'q', 'k'},
    };

    std::string fen;

    for (int y = 7; y >= 0; --y) {
        int num_empty = 0;

        for (int x = 0; x < 8; ++x) {
       //     const auto sq = Square{x, y};
       //     const auto bb = Bitboard{sq};
       //     const auto piece = pos.piece_on(sq);

//            if (piece == PieceType::NO_TYPE) {
//                num_empty++;
//            } else {
//                // Add the number of empty squares so far
//                if (num_empty > 0) {
//                    fen += std::to_string(num_empty);
//                }
//                num_empty = 0;

//                const auto gg = static_cast<bool>(pos.occupancy(Color::BLACK) & bb);
//                fen += piece_chars[gg][piece];
//            }
        }

        // Add the number of empty squares when we reach the end of the rank
        if (num_empty > 0) {
            fen += std::to_string(num_empty);
        }

        if (y > 0) {
            fen += "/";
        }
    }

    return fen;
}

[[nodiscard]] std::string fen_side(const Board &pos) noexcept {
    return pos.turn() == Color::WHITE ? "w" : "b";
}

[[nodiscard]] std::string fen_castling(const Board &pos) noexcept {
    std::string part;
//ZZ    if (pos.can_castle(Color::WHITE, MoveType::ksc)) {
//        part += "K";
//    }
//    if (pos.can_castle(Color::WHITE, MoveType::qsc)) {
//        part += "Q";
//    }
//    if (pos.can_castle(Color::BLACK, MoveType::ksc)) {
//        part += "k";
//    }
//    if (pos.can_castle(Color::BLACK, MoveType::qsc)) {
//        part += "q";
//    }
    if (part.empty()) {
        part = "-";
    }
    return part;
}

[[nodiscard]] std::string fen_enpassant(const Board &pos) noexcept {
//    if (pos.ep() == squares::NO_SQUARE) {
//        return "-";
//    } else {
//        return square_strings[static_cast<int>(pos.ep())];
//    }
}

[[nodiscard]] std::string fen_halfmoves(const Board &pos) noexcept {
    return std::to_string(pos.get_halfmove_clock());
}

[[nodiscard]] std::string fen_fullmoves(const Board &pos) noexcept {
    return std::to_string(pos.get_fullmove_clock());
}

[[nodiscard]] std::string Board::get_fen() const noexcept {
    std::string fen = fen_pieces(*this);
    fen += " " + fen_side(*this);
    fen += " " + fen_castling(*this);
    fen += " " + fen_enpassant(*this);
    fen += " " + fen_halfmoves(*this);
    fen += " " + fen_fullmoves(*this);
    return fen;
}

