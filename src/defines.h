#ifndef DEFINES_H
#define DEFINES_H

#include <cstdint>
#include <string>
#include <vector>
#include <atomic>

#define HASH
#define TT_XOR
#define NEW_EVAL
//#define PRETTY
//#define PVS
#define LMR

//#define DEBUG_EVAL
//#define DEBUG_LOG
//#define DEBUG_HASH
//#define DEBUG_TIME

/*******************************************************
 **	Généralités
 **---------------------------------------------------*/

using I08   = int8_t;
using U08   = uint8_t;
using I16   = int16_t;
using U16   = uint16_t;
using I32   = int32_t;
using U32   = uint32_t;
using I64   = int64_t;
using U64   = uint64_t;
using CHAR  = char;
using UCHAR = unsigned char;
using Bitboard = uint64_t;
using MOVE  = U32;

static constexpr int MAX_PLY    = 120;     // profondeur max de recherche (en demi-coups)
static constexpr int MAX_HIST   = 800;     // longueur max de la partie (en demi-coups)
static constexpr int MAX_MOVES  = 256;     // Number of moves in the candidate move array.
static constexpr int HASH_SIZE  = 16 << 20;
static constexpr int MAX_TIME   = 60*60*1000;   // 1 heure en ms

static constexpr int DEFAULT_HASH_SIZE  = 128;
static constexpr int MIN_HASH_SIZE      = 1;
static constexpr int MAX_HASH_SIZE      = 1024;

static constexpr int MAX_THREADS    = 32;

static constexpr int INVALID        = 99999;
static constexpr int INF            = 32000;
static constexpr int MATE           = 30000;
static constexpr int MAX_EVAL       = MATE - MAX_PLY;
static constexpr int TB_WIN_BOUND   = MAX_EVAL - MAX_PLY;
static constexpr int INF_BOUND      = INF;

/*
 *   -INF     -MATE    -MAX_EVAL    |    MAX_EVAL     MATE     INF
 *                 xxxx                          xxxxx                      zone de mat
 */


static constexpr int LMRLegalMovesLimit = 4;
static constexpr int LMRDepthLimit = 3;

//=========================================
// FEN debug positions
const std::string empty_board     = "8/8/8/8/8/8/8/8 b - - ";
const std::string start_position  = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ";
const std::string killer_position = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
const std::string cmk_position    = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 ";
const std::string repetitions     = "2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 40 ";
const std::string START_FEN       = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const std::string KIWIPETE        = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
const std::string SILVER1         = "1rbq1rk1/1pp2pbp/p1np1np1/4p3/2PPP3/2N1BP2/PP1Q2PP/R1N1KB1R w KQ e6 ";
const std::string SILVER2         = "r1b1k2r/ppppnppp/2n2q2/2b5/3NP3/2P1B3/PP3PPP/RN1QKB1R w KQkq - 0 1";
const std::string QUIESC          = "2r1r1k1/pp1q1ppp/3p1b2/3P4/3Q4/5N2/PP2RPPP/4R1K1 w - - bm Qg4";  // test quiescence

const std::string LCTII_01        = "r3kb1r/3n1pp1/p6p/2pPp2q/Pp2N3/3B2PP/1PQ2P2/R3K2R w KQkq -";
const std::string FINE_70         = "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - -";
const std::string WAC_2           = "8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - -";

//=========================================

constexpr U64 ONE  = (U64) 1;
constexpr U64 ZERO = (U64) 0;

//=========================================================

const std::string Version = VERSION;

#ifdef HOME
const std::string Home = HOME;
#else
    const std::string Home = "./";
#endif

extern std::vector<std::string> split(const std::string& s, char delimiter);
extern void printlog(const std::string& message);

#endif // DEFINES_H
