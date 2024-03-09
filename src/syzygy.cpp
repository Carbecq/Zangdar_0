
// Fichier inspiré de Weiss et Ethereal

#include "pyrrhic/tbprobe.h"
#include "Board.h"
#include "TranspositionTable.h"
#include "Move.h"

/*
https://syzygy-tables.info/
*/


//=========================================================================
//! \brief Converts a tbresult into a score
//-------------------------------------------------------------------------
void Board::TBScore(const unsigned wdl, const unsigned dtz, int& score, int& bound) const
{
    // Convert the WDL value to a score. We consider blessed losses
    // and cursed wins to be a draw, and thus set value to zero.

    // Identify the bound based on WDL scores. For wins and losses the
    // bound is not exact because we are dependent on the height, but
    // for draws (and blessed / cursed) we know the tbresult to be exact

    switch (wdl)
    {
    case TB_WIN:
        score = TBWIN - dtz;
        bound = BOUND_LOWER;
        break;
    case TB_LOSS:
        score = -TBWIN + dtz;
        bound = BOUND_UPPER;
        break;
    default:
        score = 0;
        bound = BOUND_EXACT;
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
        || halfmove_counter
        || BB::count_bit(occupancy_all()) > TB_LARGEST)
        return false;

 //   std::cout << display() << std::endl;

 //     printf("probe_wdl : ply=%d cas=%d half=%d BC=%d-%d : ", ply, castling, halfmove_counter, BB::count_bit(occupied()), TB_LARGEST);

    // Tap into Pyrrhic's API. Pyrrhic takes the board representation, followed
    // by the enpass square (0 if none set), and the turn. Pyrrhic defines WHITE
    // as 1, and BLACK as 0, which is the opposite of how Ethereal defines them

    unsigned result = tb_probe_wdl(
        occupancy_c<WHITE>(),  occupancy_c<BLACK>(),
        occupancy_p<KING>(),   occupancy_p<QUEEN>(),
        occupancy_p<ROOK>(),   occupancy_p<BISHOP>(),
        occupancy_p<KNIGHT>(), occupancy_p<PAWN>(),
        ep_square == NO_SQUARE ? 0 : ep_square,
        turn() == WHITE ? 1 : 0);

    // Probe failed
    if (result == TB_RESULT_FAILED)
    {
 //      printf(" failed \n");
        return false;
    }

    TBScore(result, ply, score, bound);
//    printf("probe_wdl success : score=%d ply=%d tbwin=%d tbloss=%d \n", score, ply, TB_WIN, TB_LOSS);

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
    if (castling || BB::count_bit(occupancy_all()) > TB_LARGEST)
        return false;

    // Call Pyrrhic
    unsigned result = tb_probe_root(
        occupancy_c<WHITE>(),  occupancy_c<BLACK>(),
        occupancy_p<KING>(),   occupancy_p<QUEEN>(),
        occupancy_p<ROOK>(),   occupancy_p<BISHOP>(),
        occupancy_p<KNIGHT>(), occupancy_p<PAWN>(),
        halfmove_counter,
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
        return Move::CODE(from, to, pieceOn[from], pieceOn[to], NO_TYPE, Move::FLAG_NONE); // MoveMake(from, to, NORMAL_MOVE);
    else if (ep != 0u)
        return Move::CODE(from, ep_square, PAWN, PAWN, NO_TYPE, Move::FLAG_ENPASSANT);
    else /* if (promo != 0u) */
    {
        PieceType p = static_cast<PieceType>(6-promo);
        return Move::CODE(from, to, PAWN, pieceOn[to], p, Move::FLAG_NONE);
    }
}

