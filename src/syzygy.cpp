/*
  Weiss is a UCI compliant chess engine.
  Copyright (C) 2023 Terje Kirstihagen

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Fichier inspiré de Weiss

#include "pyrrhic/tbprobe.h"
#include "Board.h"
#include "TranspositionTable.h"

//=========================================================================
//! \brief Converts a tbresult into a score
//-------------------------------------------------------------------------
void Board::TBScore(const unsigned wdl, const unsigned dtz, int& score, int& bound) const
{
    switch (wdl)
    {
    case TB_WIN:
        score = TBWIN - dtz;
        bound = HASH_BETA;  //lower
        break;
    case TB_LOSS:
        score = -TBWIN + dtz;
        bound = HASH_ALPHA; //upper
        break;
    default:
        score = 0;
        bound = HASH_EXACT;
        break;
    }
}

//=========================================================================
//! \brief Probe the Win-Draw-Loss (WDL) table.
//-------------------------------------------------------------------------
bool Board::probe_wdl(int& score, int& bound, int ply) const
{
    // Don't probe at root, when castling is possible, or when 50 move rule
    // was not reset by the last move. Finally, there is obviously no point
    // if there are more pieces than we have TBs for.

    if (   ply == 0
        || castling
        || halfmove_clock
        || Bcount(occupied()) > TB_LARGEST)
        return false;

 //   std::cout << display() << std::endl;

 //     printf("probe_wdl : ply=%d cas=%d half=%d BC=%d-%d : ", ply, castling, halfmove_clock, Bcount(occupied()), TB_LARGEST);

    // Tap into Pyrrhic's API. Pyrrhic takes the board representation, followed
    // by the enpass square (0 if none set), and the turn. Pyrrhic defines WHITE
    // as 1, and BLACK as 0, which is the opposite of how Ethereal defines them

    unsigned result = tb_probe_wdl(
        occupancy_c<WHITE>(),             occupancy_c<BLACK>(),
        occupancy_p<PieceType::King>(),   occupancy_p<PieceType::Queen>(),
        occupancy_p<PieceType::Rook>(),   occupancy_p<PieceType::Bishop>(),
        occupancy_p<PieceType::Knight>(), occupancy_p<PieceType::Pawn>(),
        ep_square == NO_SQUARE ? 0 : ep_square,
        turn() == WHITE ? 1 : 0);

    // Probe failed
    if (result == TB_RESULT_FAILED)
    {
 //      printf(" failed \n");
        return false;
    }

//    printf("result=%d ply=%d tbwin=%d tbloss=%d \n", result, ply, TB_WIN, TB_LOSS);
    TBScore(result, ply, score, bound);

    //    bound = result == TB_WIN  ? HASH_BETA //BOUND_LOWER    TODO verifier
    //          : result == TB_LOSS ? HASH_ALPHA //BOUND_UPPER
    //          : HASH_EXACT; // BOUND_EXACT;
  //  printf(" success \n");

    return true;
}

//=========================================================================
//! \brief Probe Syzygy Tables at the root.
//! This function should not be used during search.
//-------------------------------------------------------------------------
bool Board::probe_root(MOVE& move) const
{
    // We cannot probe when there are castling rights, or when
    // we have more pieces than our largest Tablebase has pieces
    if (castling || Bcount(occupied()) > TB_LARGEST)
        return false;

    // Call Pyrrhic
    unsigned result = tb_probe_root(
        occupancy_c<WHITE>(),             occupancy_c<BLACK>(),
        occupancy_p<PieceType::King>(),   occupancy_p<PieceType::Queen>(),
        occupancy_p<PieceType::Rook>(),   occupancy_p<PieceType::Bishop>(),
        occupancy_p<PieceType::Knight>(), occupancy_p<PieceType::Pawn>(),
        halfmove_clock,
        ep_square == NO_SQUARE ? 0 : ep_square,
        turn() == WHITE ? 1 : 0,
        nullptr);

    // Probe failed, or we are already in a finished position.
    if (   result == TB_RESULT_FAILED
        || result == TB_RESULT_CHECKMATE
        || result == TB_RESULT_STALEMATE)
        return false;

    // Extract information
    int score;

    move = convertPyrrhicMove(result);
    unsigned wdl = TB_GET_WDL(result);
    unsigned dtz = TB_GET_DTZ(result);

    switch (wdl)
    {
    case TB_WIN:
        score = TBWIN - dtz;
        break;
    case TB_LOSS:
        score = -TBWIN + dtz;
        break;
    default:
        score = 0;
        break;
    }

    // Print thinking info
    std::cout << "info depth " << dtz
              << " score cp "  << score
              << " time 0 nodes 0 nps 0 tbhits 1 pv " << Move::name(move)
              << std::endl;
    return true;
}

MOVE Board::convertPyrrhicMove(unsigned result) const
{
    // Extract Pyrhic's move representation
    unsigned to    = TB_GET_TO(result);
    unsigned from  = TB_GET_FROM(result);
    unsigned ep    = TB_GET_EP(result);
    unsigned promo = TB_GET_PROMOTES(result);

    // Convert the move notation. Care that Pyrrhic's promotion flags are inverted
    if (ep == 0u && promo == 0u)
        return Move::CODE(from, to, cpiece[from], cpiece[to], PieceType::NO_TYPE, Move::FLAG_NONE); // MoveMake(from, to, NORMAL_MOVE);
    else if (ep != 0u)
        return Move::CODE(from, ep_square, PieceType::Pawn, PieceType::Pawn, PieceType::NO_TYPE, Move::FLAG_ENPASSANT);
    // MoveMake(from, board->epSquare, ENPASS_MOVE);
    else /* if (promo != 0u) */
    {
        PieceType p = static_cast<PieceType>(6-promo);
        return Move::CODE(from, to, PieceType::Pawn, cpiece[to], p, Move::FLAG_NONE); // MoveMake(from, to, PROMOTION_MOVE | ((4 - promo) << 14));
    }
}

