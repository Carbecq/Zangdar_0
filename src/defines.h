#ifndef DEFINES_H
#define DEFINES_H

#include <string>
#include <vector>


/*******************************************************
 **	Généralités
 **---------------------------------------------------*/

using I16   = int16_t;
using U16   = uint16_t;
using I32   = int32_t;
using U32   = uint32_t;
using I64   = int64_t;
using U64   = uint64_t;
using CHAR  = char;
using UCHAR = unsigned char;



static constexpr int BOARD_SIZE  = 128;     // nombre de cases de l'échiquier (0x88)
static constexpr int MAX_PLY     = 120;     // profondeur max de recherche (en demi-coups)  //TODO diminuer cette valeur ?
static constexpr int MAX_HIST    = 400;     // longueur max de la partie (en demi-coups)
static constexpr int NULL_MOVE_R = 2;       // réduction de la profondeur de recherche

static constexpr int MAX_MOVES  = 8192;     // Number of moves in the candidate move array.
static constexpr int MAX_SCORE  = 10000;
static constexpr int IS_MATE    = MAX_SCORE - MAX_PLY;
static constexpr int HASH_SIZE  = 16 << 20;
static constexpr int MAX_TIME   = 60*60*1000;   // 1 heure en ms

const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

enum HASH_CODE { HASH_NONE, HASH_ALPHA, HASH_BETA, HASH_EXACT };

/*******************************************************
 ** Les cases
 **---------------------------------------------------*/

enum Square {
    A1=0x00, B1, C1, D1, E1, F1, G1, H1,
    A2=0x10, B2, C2, D2, E2, F2, G2, H2,
    A3=0x20, B3, C3, D3, E3, F3, G3, H3,
    A4=0x30, B4, C4, D4, E4, F4, G4, H4,
    A5=0x40, B5, C5, D5, E5, F5, G5, H5,
    A6=0x50, B6, C6, D6, E6, F6, G6, H6,
    A7=0x60, B7, C7, D7, E7, F7, G7, H7,
    A8=0x70, B8, C8, D8, E8, F8, G8, H8,
    OFFBOARD
};


static const std::vector<Square> SQUARES = {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    OFFBOARD
};

static const std::vector<Square> MIRROR_SQUARES = {
    A8, B8, C8, D8, E8, F8, G8, H8, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD,
    A7, B7, C7, D7, E7, F7, G7, H7, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD,
    A6, B6, C6, D6, E6, F6, G6, H6, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD,
    A5, B5, C5, D5, E5, F5, G5, H5, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD,
    A4, B4, C4, D4, E4, F4, G4, H4, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD,
    A3, B3, C3, D3, E3, F3, G3, H3, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD,
    A2, B2, C2, D2, E2, F2, G2, H2, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD,
    A1, B1, C1, D1, E1, F1, G1, H1, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD, OFFBOARD
};

enum File {
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H
};

static const std::vector<File> FILES = {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};

enum Rank {
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8
};

static const std::vector<Rank> RANKS = {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8
};


/*******************************************************
 ** Les couleurs
 **---------------------------------------------------*/
enum Color  {
    WHITE = 0,
    BLACK = 1,
    BOTH
};

static const std::vector<Color> COLORS = { WHITE, BLACK };

constexpr Color operator~(Color c) { return Color(c ^ 1); }

/*******************************************************
 ** Les pieces
 **---------------------------------------------------*/
enum PieceType {
    EMPTY   = 0,
    PAWN    = 1,
    KNIGHT  = 2,
    BISHOP  = 3,
    ROOK    = 4,
    QUEEN   = 5,
    KING    = 6
};

// static const std::string NomPiece[7] = {"VIDE", "PION", "CAVA", "FOU", "TOUR", "DAME", "ROI"};

/*******************************************************
 ** Droit au roque
 **---------------------------------------------------*/
enum Castle {
    CASTLE_NONE = 0,
    CASTLE_WK   = 1,
    CASTLE_WQ   = 2,
    CASTLE_BK   = 4,
    CASTLE_BQ   = 8
};

/*******************************************************
 ** Coup Candidat
 **---------------------------------------------------*/

// https://www.chessprogramming.org/Encoding_Moves
// voir TSCP

enum CandidateMoveFlags {
    CMF_NONE        = 0,                // un coup qui n'existe pas.
 //                     12345678123456781234567812345678
 //                       098765432109876543210987654321
    CMF_FROM        = 0b00000000000000000000000001111111,       // from
    CMF_DEST        = 0b00000000000000000011111110000000,       // dest
    CMF_TYPE        = 0b00000000000000011100000000000000,       // type de la pièce jouant
    CMF_PROMOTION   = 0b00000000000011100000000000000000,       // type de la pièce de promotion

    CMF_CAPTURE     = 0b00000000000000001,       // toute capture (incluant en-passant)
    CMF_PAWN2       = 0b00000000000000010,       // avance de 2 cases d'un pion
    CMF_PEP         = 0b00000000000000100,       // capture en-passant
    CMF_CASTLE_WK   = 0b00000000000001000,       // petit roque blanc
    CMF_CASTLE_WQ   = 0b00000000000010000,       // grand roque blanc
    CMF_CASTLE_BK   = 0b00000000000100000,       // petit roque noir
    CMF_CASTLE_BQ   = 0b00000000001000000,       // grand roque noir

    CMF_CASTLE_W    = CMF_CASTLE_WK | CMF_CASTLE_WQ ,           // roque blanc
    CMF_CASTLE_B    = CMF_CASTLE_BK | CMF_CASTLE_BQ ,           // roque noir
    CMF_CASTLE      = CMF_CASTLE_W  | CMF_CASTLE_B  ,           // roque

    CMF_FLAGS       = CMF_CAPTURE | CMF_PAWN2 | CMF_PEP | CMF_CASTLE

};

//*******************************************************
//  Valeur des pièces
//  Values in centi-pawns of the pieces.
//  valeurs un peu différentes de Gerbil, à voir ...
//-------------------------------------------------------
enum PieceValue {
    valEMPTY  =    0,
    valPAWN   =  100,
    valKNIGHT =  325,
    valBISHOP =  325,
    valROOK	  =  550,
    valQUEEN  = 1000,
    valKING   =    0	// il y a toujours un roi
};

static const std::vector<PieceValue> PIECEVALUES = {
    valEMPTY,
    valPAWN,
    valKNIGHT,
    valBISHOP,
    valROOK,
    valQUEEN,
    valKING
};


extern std::string VERSION;
extern std::string ascii(Square s);

extern std::vector<std::string> split(const std::string& s, char delimiter);

//extern FILE* fp;


#endif // DEFINES_H
