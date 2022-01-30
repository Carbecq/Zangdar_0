#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <iostream>
#include <random>
#include <array>

#include "defines.h"
#include "Position.h"
#include "Piece.h"

//TODO faut-il utiliser un générateur avec la même seed ?

class Zobrist
{
    private:
        U64 gen_hash();
        std::mt19937_64 generator;

    public:
        U64 piece_key[2][7][BOARD_SIZE];
        U64 side_key;
        U64 castle_key[16];
        U64 en_passant_key[BOARD_SIZE];


        Zobrist();
        void set_key(U64& key, const Position* pos, const std::array<Piece *, BOARD_SIZE> &board);

        void change_side(U64& h) {
            h ^= side_key;
        };
        void update_piece(U64& h, Color c, PieceType t, Square s) {
            h ^= piece_key[c][t][s];
        };
        void update_castle(U64& h, U32 castle) {
            h ^= castle_key[castle];
        };
        void update_en_passant(U64& h, Square s) {
           h ^= en_passant_key[s];
        };
        friend std::ostream& operator<<(std::ostream& out, const Zobrist& zobrist);
};

#endif // ZOBRIST_H
