#include <iostream>
#include <cassert>

#include "defines.h"
#include "Position.h"
#include "ChessBoard.h"
#include "Zobrist.h"

// https://www.cplusplus.com/reference/random/mt19937_64/
// https://mediocrechess.blogspot.com/2007/01/guide-zobrist-keys.html
// https://www.chessprogramming.org/Zobrist_Hashing
// http://www.seanet.com/~brucemo/topics/hashing.htm
// https://talkchess.com/forum3/viewtopic.php?f=7&t=76679&hilit=zobrist+seed&sid=309ea700683736dd964a3469fd7802c0&start=10

Zobrist::Zobrist()
{
    // use deterministic seed, any 64-bit constant should do
    std::mt19937_64 generator(0x1234);
    std::uniform_int_distribution<U64> dist(ZERO, ULLONG_MAX);

    // Initialisation

    // le camp
    side_key = dist(generator);

    // les pièces
    for (int c = 0; c < 2; ++c) {
        for (int t = 0; t < 7; ++t) {
            for (int s = 0; s < BOARD_SIZE; ++s) {
                piece_key[c][t][s] = dist(generator);
            }
        }
    }

    // le droit au roque
    for (int i = 0; i < 4; ++i) {
        castle_key[i] = dist(generator);
    }

    // la prise en passant
    for (int i = 0; i < BOARD_SIZE; ++i) {
        en_passant_key[i] = dist(generator);
    }
}

//=================================================================
//! \brief  Génère une clef unique définissant la position
//!
//! Une position est définie par
//!     + le camp au trait
//!     + les pièces
//!     + la prise en passant
//!     + le droit au roque
//-----------------------------------------------------------------
void Zobrist::set_key(U64& key, const Position *pos, const std::array<Piece*, BOARD_SIZE>& board)
{
    PieceType type = EMPTY;
    Color color;

    // les pièces
    for(int sq = 0; sq < BOARD_SIZE; ++sq)
    {
        if (!(sq & 0x88))
        {
            Piece* piece = board[sq];
            assert(piece != nullptr);

            type  = piece->type();
            color = piece->color();

            if (type != EMPTY)
            {
                key ^= piece_key[color][type][sq];
            }
        }
    }

    // le camp au trait
    // hash the side only if black is to move
    if(pos->side_to_move == BLACK)
        key ^= side_key;

    // la prise en passant
    // if enpassant square is on board
    if(pos->ep_square != OFFBOARD)
    {
        assert(pos->ep_square >= 0 && pos->ep_square < BOARD_SIZE);
        key ^= en_passant_key[pos->ep_square];
    }

    // le roque
    assert(pos->castle >= 0 && pos->castle <= 15);
    key ^= castle_key[pos->castle];


}
