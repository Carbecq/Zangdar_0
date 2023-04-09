#include "Board.h"



//============================================================
//! \brief  Retourne la chaine fen correspondant
//! à la position
//------------------------------------------------------------
[[nodiscard]] std::string Board::get_fen() const noexcept
{
    std::string fen;

    for (int y = 7; y >= 0; --y)
    {
        int num_empty = 0;

        for (int x = 0; x < 8; ++x)
        {
            const auto sq = Square::square(x, y);
            const PieceType piece = piece_on(sq);
            if (piece == PieceType::NO_TYPE) {
                num_empty++;
            } else {
                // Add the number of empty squares so far
                if (num_empty > 0) {
                    fen += std::to_string(num_empty);
                }
                num_empty = 0;

                fen += piece_symbol[color_on(sq)][piece];
            }
        }

        // Add the number of empty squares when we reach the end of the rank
        if (num_empty > 0) {
            fen += std::to_string(num_empty);
        }

        if (y > 0) {
            fen += "/";
        }
    }

    //------------------------------------------------------- side

    fen += (turn() == Color::WHITE) ? " w" : " b";

    //------------------------------------------------------- castling

    std::string part;
    if (can_castle_k<WHITE>())
        part += "K";
    if (can_castle_q<WHITE>())
        part += "Q";
    if (can_castle_k<BLACK>())
        part += "k";
    if (can_castle_q<BLACK>())
        part += "q";
    if (part.empty())
        part = "-";

    fen += " ";
    fen += part;

    //------------------------------------------------------- en-passant

    fen += " ";
    if (ep_square == NO_SQUARE)
        fen += "-";
    else
        fen += square_name[ep_square];

    //------------------------------------------------------- half-move

    fen += " ";
    fen += std::to_string(get_halfmove_clock());

    //------------------------------------------------------- full-move

    fen += " ";
    fen += std::to_string(get_fullmove_clock());

    return fen;
}

