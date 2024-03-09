#ifndef EVALUATE_H
#define EVALUATE_H

#include "defines.h"
#include "types.h"

// Valeur des pièces
//  mg = middle game
//  eg = end game

// Idée de Stockfish, reprise par de nombreux codes

/// Score enum stores a middlegame and an endgame value in a single integer (enum).
/// The least significant 16 bits are used to store the middlegame value and the
/// upper 16 bits are used to store the endgame value. We have to take care to
/// avoid left-shifting a signed int to avoid undefined behavior.

#define MakeScore(mg, eg) ((int)((unsigned int)(eg) << 16) + (mg))
#define S(mg, eg) MakeScore((mg), (eg))
#define MgScore(s) ((int16_t)((uint16_t)((unsigned)((s)))))
#define EgScore(s) ((int16_t)((uint16_t)((unsigned)((s) + 0x8000) >> 16)))


struct EvalInfo {
    Bitboard occupied;
    Bitboard pawns[N_COLORS];
    Bitboard knights[N_COLORS];
    Bitboard bishops[N_COLORS];
    Bitboard rooks[N_COLORS];
    Bitboard queens[N_COLORS];
    Bitboard passedPawns;

    Bitboard pawnAttacks[N_COLORS];
    Bitboard mobilityArea[N_COLORS];
    Bitboard enemyKingZone[N_COLORS];

    int attackPower[N_COLORS] = {0, 0};
    int attackCount[N_COLORS] = {0, 0};

    int phase24;

};

enum PieceValueEMG {
    P_MG =  104, P_EG =  205,
    N_MG =  408, N_EG =  625,
    B_MG =  413, B_EG =  653,
    R_MG =  558, R_EG = 1102,
    Q_MG = 1479, Q_EG = 1945,
    K_MG =    0, K_EG =    0
};

enum Phase { MG, EG };

//-----------------------------------------------------------------
//  Tables PeSTO
//  Valeurs de Weiss
//  Re-tunées ensuite
//-----------------------------------------------------------------
//http://www.talkchess.com/forum3/viewtopic.php?f=2&t=68311

//  Valeur des pièces
constexpr int MGPieceValue[N_PIECES] = {
    0, P_MG, N_MG, B_MG, R_MG, Q_MG, K_MG
};
constexpr int EGPieceValue[N_PIECES] = {
    0, P_EG, N_EG, B_EG, R_EG, Q_EG, K_EG
};

constexpr Score PawnValue   = S(P_MG, P_EG);
constexpr Score KnightValue = S(N_MG, N_EG);
constexpr Score BishopValue = S(B_MG, B_EG);
constexpr Score RookValue   = S(R_MG, R_EG);
constexpr Score QueenValue  = S(Q_MG, Q_EG);
constexpr Score KingValue   = S(K_MG, K_EG);

//========================================================== DEBUT TUNER

//----------------------------------------------------------
//  Bonus positionnel des pi├¿ces
//  Du point de vue des Blancs - Mes tables sont comme ├ºa

constexpr Score PawnPSQT[N_SQUARES] = {
    S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0),
    S( -13,   10), S( -14,    8), S( -21,   12), S( -11,   10), S( -17,   31), S(  19,   17), S(  32,   -4), S(   3,  -24),
    S( -25,    4), S( -35,   -3), S( -30,   -3), S( -25,   -9), S( -15,    1), S(  -7,    4), S(   2,  -16), S(  -5,  -15),
    S( -18,   19), S( -27,   13), S( -13,  -12), S( -15,  -16), S(  -3,  -16), S(   4,  -14), S(  -4,   -8), S(  -4,   -6),
    S( -11,   40), S( -11,   19), S( -10,    6), S(   4,  -25), S(  23,  -15), S(  39,  -17), S(  10,    8), S(   2,   16),
    S(   2,   74), S(  16,   77), S(  33,   53), S(  39,   28), S(  62,   31), S( 133,   32), S(  91,   60), S(  33,   66),
    S(  57,    3), S(  51,   32), S(  32,   64), S(  73,   35), S(  82,   40), S(  88,   27), S( -40,   85), S( -52,   53),
    S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0),
};

constexpr Score KnightPSQT[N_SQUARES] = {
    S( -88,  -61), S( -15,  -52), S( -40,  -19), S(  -7,    7), S(   0,    6), S(  -5,  -26), S( -13,  -22), S( -54,  -37),
    S( -34,  -37), S( -40,   -3), S( -19,  -16), S(  -5,   10), S( -10,    6), S( -10,  -16), S( -22,  -15), S(  -6,   -6),
    S( -22,  -44), S(  -5,   -5), S(  -1,   15), S(  15,   39), S(  16,   37), S(   8,   14), S(  11,    1), S(  -1,  -17),
    S(   0,    2), S(  18,   12), S(  23,   56), S(  27,   58), S(  25,   59), S(  36,   54), S(  54,   13), S(  31,   13),
    S(   9,   -3), S(  25,   18), S(  48,   56), S(  43,   73), S(  29,   71), S(  69,   48), S(  30,   25), S(  39,   -3),
    S( -21,  -13), S(  26,    6), S(  36,   53), S(  55,   53), S(  98,   31), S(  80,   36), S(  45,   -6), S(   3,  -19),
    S(   0,  -26), S(   0,   -1), S(  51,  -12), S(  57,   27), S(  65,   10), S(  79,  -32), S( -12,    2), S(  16,  -41),
    S(-207,  -71), S(-112,  -12), S(-145,   34), S( -53,    8), S(  -1,   16), S(-128,   43), S( -79,  -11), S(-158, -115),
};

constexpr Score BishopPSQT[N_SQUARES] = {
    S(  34,  -17), S(  40,   -2), S(  15,    7), S(  -4,    5), S(   9,    6), S(  12,   -2), S(  22,   -8), S(  33,  -24),
    S(  25,  -13), S(  22,  -31), S(  26,  -12), S(   2,    5), S(   3,    6), S(  10,  -13), S(  29,  -30), S(  29,  -53),
    S(  12,   -8), S(  32,   11), S(  22,   14), S(  16,   21), S(  21,   25), S(  27,   13), S(  35,    1), S(  37,   -8),
    S(  15,   -7), S(  21,    2), S(  23,   29), S(  39,   30), S(  34,   31), S(  25,   21), S(  25,   10), S(  40,   -8),
    S(  -5,   17), S(  40,   19), S(  30,   20), S(  58,   39), S(  44,   36), S(  38,   27), S(  45,   19), S(  -1,   23),
    S(   4,   25), S(  31,   24), S(  57,   24), S(  39,   18), S(  49,   22), S(  57,   33), S(  27,   33), S(  12,   18),
    S( -21,   23), S(  23,   29), S(   8,   28), S( -23,   36), S(   7,   23), S(  -7,   33), S( -18,   37), S( -49,   31),
    S( -40,   51), S( -55,   44), S(-133,   58), S(-125,   63), S(-128,   57), S(-150,   49), S( -25,   25), S( -39,   24),
};

constexpr Score RookPSQT[N_SQUARES] = {
    S( -23,   26), S( -22,   25), S( -20,   28), S( -10,   12), S( -11,   10), S(  -9,   19), S(  -3,   14), S( -19,   10),
    S( -49,   21), S( -26,   19), S( -17,   22), S( -13,   14), S(  -9,    7), S( -18,    0), S(  -7,   -6), S( -44,   12),
    S( -30,   20), S( -22,   34), S( -30,   29), S( -23,   21), S( -22,   21), S( -22,   11), S(   7,    6), S( -18,    5),
    S( -23,   43), S( -21,   60), S( -20,   58), S( -12,   48), S( -16,   44), S( -20,   45), S(   5,   36), S( -12,   30),
    S(  -8,   64), S(  14,   58), S(  25,   60), S(  47,   48), S(  41,   44), S(  46,   36), S(  44,   32), S(  25,   43),
    S(  -3,   68), S(  50,   45), S(  26,   64), S(  53,   46), S(  70,   33), S(  65,   44), S(  89,   18), S(  30,   46),
    S(  -1,   74), S( -17,   84), S(  11,   82), S(  18,   85), S(   8,   83), S(  19,   52), S(   4,   62), S(  27,   51),
    S(  36,   67), S(  36,   78), S(  -3,   98), S(   4,   88), S(  14,   88), S(  15,   88), S(  34,   83), S(  50,   75),
};

constexpr Score QueenPSQT[N_SQUARES] = {
    S(   2,  -44), S(  -6,  -47), S(  -2,  -42), S(   7,  -44), S(   6,  -47), S( -16,  -53), S(  -1,  -83), S(  12,  -57),
    S(  10,  -37), S(  15,  -28), S(  20,  -40), S(  15,   -3), S(  16,  -13), S(  17,  -95), S(  29, -125), S(  23,  -78),
    S(   6,  -26), S(  19,    3), S(   9,   27), S(   4,   21), S(   8,   24), S(   8,   27), S(  35,   -6), S(  28,  -13),
    S(  12,   -3), S(  10,   45), S(   1,   50), S(  -6,   95), S(  -8,  103), S(  11,   87), S(  27,   72), S(  39,   76),
    S(   3,   17), S(   2,   60), S( -11,   70), S( -15,  130), S( -12,  167), S(   0,  182), S(  40,  166), S(  18,  133),
    S( -15,   44), S(   0,   33), S( -10,   79), S( -15,  122), S(   7,  164), S(  42,  167), S(  63,  126), S(  14,  149),
    S( -18,   57), S( -60,   99), S( -29,  105), S( -84,  199), S( -80,  239), S( -15,  179), S( -43,  165), S(  16,  150),
    S( -32,   87), S( -10,  101), S(  -5,  128), S(  10,  135), S(   7,  157), S(  45,  146), S(  43,  134), S(  31,  125),
};

constexpr Score KingPSQT[N_SQUARES] = {
    S(  40,  -41), S(  90,   12), S(  55,   41), S( -35,   34), S(  29,    5), S( -30,   60), S(  70,   12), S(  47,  -51),
    S(  80,   33), S(  80,   53), S(  61,   72), S(   5,   88), S(  22,   87), S(  34,   86), S(  72,   60), S(  63,   27),
    S(  37,   30), S( 119,   56), S(  93,   87), S(  71,  109), S(  92,  105), S(  79,   99), S(  91,   68), S(   7,   52),
    S(  29,   30), S( 122,   74), S( 129,  113), S(  45,  151), S(  90,  140), S( 125,  115), S( 123,   81), S( -36,   61),
    S(  38,   67), S( 109,   98), S( 103,  138), S(  65,  158), S(  78,  154), S( 128,  130), S( 118,  103), S(   2,   72),
    S(  35,   64), S( 133,  110), S( 120,  132), S(  92,  137), S( 128,  131), S( 162,  138), S( 147,  124), S(  45,   64),
    S(  -6,   29), S(  54,  110), S(  55,  112), S(  78,   99), S(  81,  100), S(  89,  115), S(  95,  132), S(  42,   45),
    S( -24,  -67), S(  25,   18), S(  11,   45), S(   9,   83), S(   0,   59), S(  25,   71), S(  43,   88), S(  15,  -58),
};

//----------------------------------------------------------
// Pawn bonuses and maluses
constexpr Score PawnDoubled = S(-13,-42);
constexpr Score PawnSupport = S( 20, 11);
constexpr Score PawnOpen    = S(-14,-17);
constexpr Score PawnPhalanx[N_RANKS] = {
    S(  0,  0), S(  8, -3), S( 19,  7), S( 24, 33),
    S( 60,126), S(171,248), S(169,318), S(  0,  0)
};
constexpr Score PawnIsolated = S( -8,-19);
constexpr Score PawnPassed[N_RANKS] = {
    S(  0,  0), S(-15, 36), S(-13, 43), S(-70,128),
    S(-13,161), S(107,200), S(278,233), S(  0,  0)
};
constexpr Score PassedDefended[N_RANKS] = {
    S(  0,  0), S(  0,  0), S(  5,-14), S( -2,-14),
    S(  4, 15), S( 49, 64), S(161, 68), S(  0,  0)
};

//----------------------------------------------------------
// Pions pass├®s
constexpr Score PassedDistUs[4] = {
    S( 16,-29), S( 10,-37), S( -8,-35), S(-13,-27)
};
constexpr Score PassedDistThem =S( -3, 19);
constexpr Score PassedBlocked[4] = {
    S( -1,-23), S(  4,-34), S(  9,-93), S(-28,-121)
};

//----------------------------------------------------------
// Misc Bonus
constexpr Score MinorBehindPawn =S(  7, 42);

constexpr Score KnightOutpost[2] = { S(  19,  -28), S(  41,   17) };
constexpr Score BishopPair = S( 25,124);
constexpr Score BishopBadPawn = S(  -1,   -5);
constexpr Score OpenForward =  S( 29, 20);
constexpr Score SemiForward = S( 10, 18);
constexpr Score KingLineDanger[28] = {
    S(  0,  0), S(  0,  0), S(  0,  0), S(-11, 41),
    S(-27, 41), S(-31, 29), S(-30, 25), S(-37, 34),
    S(-41, 33), S(-54, 39), S(-50, 34), S(-65, 43),
    S(-68, 42), S(-79, 43), S(-88, 45), S(-86, 43),
    S(-98, 44), S(-100, 37), S(-107, 33), S(-112, 29),
    S(-119, 27), S(-133, 18), S(-131,  9), S(-157, -3),
    S(-140,-18), S(-132,-33), S(-130,-32), S(-135,-34)
};
constexpr Score KingAtkPawn = S(  3, 54);

//----------------------------------------------------------
// Menaces
constexpr Score PawnThreat =S( 52, 73);
constexpr Score PushThreat = S( 18,  7);

//----------------------------------------------------------
// Mobilit├® :
constexpr Score KnightMobility[9] = {
    S(-35,-142), S(-28,-19), S( -7, 46), S(  3, 81), S( 13, 92), S( 16,111), S( 23,111), S( 32,106),
    S( 46, 77)
};

constexpr Score BishopMobility[14] = {
    S(-40,-93), S(-21,-54), S( -9,  6), S( -2, 44), S(  8, 61), S( 17, 89), S( 22,107), S( 22,114),
    S( 20,126), S( 26,126), S( 31,125), S( 55,107), S( 51,119), S(103, 71)
};

constexpr Score RookMobility[15] = {
    S(-106,-146), S(-22,  4), S( -5, 70), S( -3, 77), S( -3,113), S(  2,131), S(  0,151), S(  7,155),
    S( 12,162), S( 20,168), S( 28,174), S( 30,178), S( 31,182), S( 45,172), S( 86,140)
};

constexpr Score QueenMobility[28] = {
    S(-63,-48), S(-96,-54), S(-91,-107), S(-21,-129), S(  3,-58), S( -1, 65), S( -1,135), S(  2,177),
    S(  5,204), S(  9,224), S( 11,241), S( 14,256), S( 17,260), S( 17,269), S( 18,276), S( 19,282),
    S( 17,290), S( 15,295), S( 11,302), S(  9,310), S( 17,303), S( 13,307), S( 38,287), S( 49,268),
    S(116,213), S(130,187), S(143,163), S(123,160)
};




//============================================================== FIN TUNER

//  Sécurité du Roi
constexpr Score AttackPower[7]   = { 0, 0,  35, 20, 40, 80, 0 };
constexpr Score CheckPower[7]    = { 0, 0, 100, 35, 65, 65, 0 };
constexpr Score CountModifier[8] = { 0, 0,  64, 96, 113, 120, 124, 128 };

//------------------------------------------------------------
//  Bonus car on a le trait
constexpr int Tempo = 15;


#endif // EVALUATE_H
