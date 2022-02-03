#ifndef BOARD_H
#define BOARD_H

//  Classe contenant les pièces de l'échiquier
//      + l'échiquier est de forme 0x88
//        il contient des pointeurs sur la liste des pièces

class Board;

#include "defines.h"
#include "Piece.h"
#include "Move.h"
#include "Position.h"
#include "Zobrist.h"


// tables de déplacement

const int  delta_knight[8] = { 14,  31,  33,  18, -14, -31, -33, -18    };
const int  delta_bishop[4] = { 17,  15, -17, -15                        };
const int  delta_rook[4]   = { 16,   1,  -1, -16                        };
const int  delta_queen[8]  = { -1,   1,  16, -16,  15,  17, -15, -17    };
const int  delta_king[8]   = {-17, -16, -15,  -1,   1,  15,  16,  17    };
const int  delta_wpawn[2]  = { 15,  17                                  };  // uniquement les déplacements de prise
const int  delta_bpawn[2]  = {-15, -17                                  };

/* This is the castle_mask array. We can use it to determine
the castling permissions after a move. What we do is
logical-AND the castle bits with the castle_mask bits for
both of the move's squares. Let's say castle is 1, meaning
that white can still castle kingside. Now we play a move
where the rook on h1 gets captured. We AND castle with
castle_mask[63], so we have 1&14, and castle becomes 0 and
white can't castle kingside anymore.
 (TSCP) */

const U32 castle_mask[128] = {
    13, 15, 15, 15, 12, 15, 15, 14,   15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,   15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,   15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,   15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,   15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,   15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,   15, 15, 15, 15, 15, 15, 15, 15,
     7, 15, 15, 15,  3, 15, 15, 11,   15, 15, 15, 15, 15, 15, 15, 15
};

// Flags permettant de connaitre quelle pièce peut attaquer une case donnée
enum AttackPieceFlags {
    APF_NONE	=	0x00,
    APF_WPAWN	=	0x01,
    APF_BPAWN	=	0x02,
    APF_KNIGHT  =	0x04,
    APF_BISHOP  =	0x08,
    APF_ROOK	=	0x10,
    APF_QUEEN	=	0x20,
    APF_KING	=	0x40
};

class Board
{
public:
    Board();
    ~Board();

    void reset();
    void tests();
    void init();

    void gen_moves();
    void gen_wmoves(int& index);
    void gen_bmoves(int& index);

    void gen_caps();
    void gen_wcaps(int& index);
    void gen_bcaps(int& index);

    void init_fen(const std::string& fen, bool logTactics);
    void mirror_fen(const std::string& fen);
    void display_ascii() const;
    void setOutput(int m) { output = m; }

    I32  evaluate(Color side);
    bool is_in_check(const Color c);

    // Makemove.cpp
    bool make_move(const Move *move);
    void unmake_move(const Move *move);
    void make_null_move();
    void take_null_move();
    void pv_move(U32 PvMove, int ply);
    int  nbr_pieces(Color side) const;

    // Tests.cpp
    U64  perft(int depth);
    U64  divide(int depth);


protected:
    std::array<Piece*, BOARD_SIZE>  board;          // pointeurs sur la liste des pièces
    std::vector<Piece>              pieces[2];      // liste des pièces par couleur
    std::array<Move, MAX_MOVES>     moves;          // liste des coups
    std::array<int, MAX_PLY>        first_move;     // indice du premier coup dans 'moves'
    std::array<History, MAX_HIST>   history;        // données à conserver pour 'unmake_move'
    int                             output;

    //TODO en principe, il faudrait les mettre dans Search
    int     searchHistory[2][7][BOARD_SIZE];
    U32     searchKillers[2][MAX_PLY];      // Killer Moves, 2 pour chaque profondeur de recherche

    Position*                       positions;      // données de la position

private:
    std::array<Piece, BOARD_SIZE>   vide;           // pièces "vide", évite les tests sur les pointeurs null
    Zobrist*                        zobrist;        // classe de gestion des clef Zobrist

    // tableau d'attaque et de déplacement
    U32 attack_array[256]; // 240 ??? ou 238 ??
    int  delta_array[256];



    //   static File file(const Square s) {
    //       return File(s & 7);
    //   }
    //   static Rank rank(const Square s) {
    //       return Rank(s >> 4);
    //   }

    // note de programmation : à l'heure actuelle , remplacer les multiplcations et le divisions
    //                         par des décalages est inutile. le compilo fait ça très bien
/*
 * sq0x88 = 16 * rank07 + file07;
 * file07 = sq0x88 & 7;
 * rank07 = sq0x88 >> 4; // sq0x88 / 16
 *
 * sq0x88 = sq8x8 + (sq8x8 & ~7);
 *
 * sq8x8 = (sq0x88 + (sq0x88 & 7)) >> 1;
 *
 */

    /* generate square number from row and column */
    #define SQUARE0x88(row,col) (row * 16 + col)

    /* does a given number represent a square on the board? */
    #define ON_BOARD(x)  ( (x) & 0x88 ) ? (false) : (true)

    /* get board column that a square is part of */
    #define FILE07(sq)  ( (sq) & 7 )

    /* get board row that a square is part of */
    #define RANK07(sq)  ( (sq) >> 4 )



    File    file(const Square s)               const {
        return File((s) & 7);
    }
    Rank    rank(const Square s)               const {
        return Rank((s) / 16);
    }
    Square  square(const Rank r, const File f) const {
        return Square((r) * 16 + (f));
    }

    bool is_empty(const Square s)   const {
        return board[s]->type() == EMPTY;
    }
    bool is_out(const Square s)     const {
        return (s) & 0x88;
    }
    bool is_dark(const Square s)    const {
        return (s & 7) % 2 == (s / 16) % 2;
    }

    Piece* operator[] (const Square s)              {
        return board[s];
    }
    const Piece* operator[] (const Square s) const  {
        return board[s];
    }
    Square incr(const Square s) {
        return(static_cast<Square>(s + 1));
    }


    void gen_wpawn(Square from, int& index);
    void gen_bpawn(Square from, int& index);
    void gen_bishop(Color c, Square from, int& index);
    void gen_rook(Color c, Square from, int& index);
    void gen_wking(Square from, int& index);
    void gen_bking(Square from, int& index);
    void gen_queen(Color c, Square from, int& index);
    void gen_knight(Color c, Square from, int& index);
    void gen_slider(Color c, Square from, const int delta[], int& index);

    void gen_wpawn_caps(Square from, int& index);
    void gen_bpawn_caps(Square from, int& index);
    void gen_bishop_caps(Color c, Square from, int& index);
    void gen_rook_caps(Color c, Square from, int& index);
    void gen_wking_caps(Square from, int& index);
    void gen_bking_caps(Square from, int& index);
    void gen_queen_caps(Color c, Square from, int& index);
    void gen_knight_caps(Color c, Square from, int& index);
    void gen_slider_caps(Color c, Square from, const int delta[], int& index);

    void add_quiet_move(Square from, Square to, PieceType promo, U32 flags, int& index);
    void add_capture_move(Square from, Square to, PieceType promo, U32 flags, int& index);
    void add_enpassant_move(Square from, Square to, U32 flags, int &index);
    void add_quiet_promote(Square from, Square to, U32 flags, int& index);
    void add_capture_promote(Square from, Square to, U32 flags, int& index);

    void init_MVVLVA(void);

    // Attack.cpp
    void init_ray(int nbr_depl, int delta, U32 flag);
    void init_attack();
    bool is_attacked_by(const Square s, const Color c) const;


    //------------------------Evaluation.cpp
    void flip();
    bool MaterialDraw() const;

    bool can_attack(const PieceType t, const Square from, const Square to) const;
    void verif(const std::string &msg, Position *pos);

    // Fen.cpp
    void add_piece(PieceType t, Color c, Square s);
    void update();


};

#endif // BOARD_H
