#ifndef DEFINES_H
#define DEFINES_H

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <bitset>

// ne pas oublier la version dans Zangdar.pro

#define USE_HASH

#define USE_RAZORING
#define USE_REVERSE_FUTILITY_PRUNING
#define USE_NULL_MOVE_PRUNING
#define USE_PROBCUT
#define USE_INTERNAL_ITERATIVE_DEEPENING
//#define USE_FUTILITY_PRUNING
#define USE_LATE_MOVE_PRUNING
#define USE_SINGULAR_EXTENSION
#define USE_LATE_MOVE_REDUCTION

#define USE_TC_WEISS

//#define USE_ACC

// NE PAS UTILISER PRETTY avec
//      + Arena (score mal affiché)
//      + test STS
//#define USE_PRETTY

#define TT_SUNGORUS

/*******************************************************
 **	Généralités
 **---------------------------------------------------*/

using I08   = int8_t;
using U08   = uint8_t;
using I16   = int16_t;          // −32768 to 32767
using U16   = uint16_t;         // 0 to 65535
using I32   = int32_t;
using U32   = uint32_t;
using I64   = int64_t;
using U64   = uint64_t;
using CHAR  = char;
using UCHAR = unsigned char;

using Bitboard  = U64;
using MOVE      = U32;

using Score = int;  // Valeur spéciale contenant à la fois mg et eg

static constexpr int MAX_PLY    = 128;     // profondeur max de recherche (en demi-coups)
static constexpr int MAX_HIST   = 800;     // longueur max de la partie (en demi-coups)
static constexpr int MAX_MOVES  = 400;     // Number of moves in the candidate move array.
static constexpr int MAX_TIME   = 60*60*1000;   // 1 heure en ms

static constexpr int HASH_SIZE      = 128;      // en Mo , 128 ?
static constexpr int MIN_HASH_SIZE  = 1;
static constexpr int MAX_HASH_SIZE  = 1024;
static constexpr int PAWN_HASH_SIZE = 64;       // en Ko

static constexpr int MAX_THREADS    = 32;

static constexpr int MATE           = 31000;
static constexpr int MATE_IN_X      = MATE - MAX_PLY;
static constexpr int TBWIN          = 30000;
static constexpr int TBWIN_IN_X     = TBWIN - MAX_PLY;

static constexpr int INFINITE       = MATE + 1;
static constexpr int NOSCORE        = MATE + 2;     // ne peut jamais être atteint


/*                                           29872          30000     30872         31000    31001  : moi
 *                                           39872          30000     30001         30129    30130  : weiss 0.10
 *   -INFINITE     -MATE    -MATE_IN_X    |  TBWIN_IN_X.....TBWIN.....MATE_IN_X.....MATE.....INFINITE
 *                 xxxx                                                        xxxxx                      zone de mat
 */

/*  Explications
 *
 *   MATE et MATE_IN_X servent à repérer et afficher les mats
 *   Dans le cas d'un mat, il faut avoir un score indépendant de la profondeur de recherche : score = -MATE + ply
 *   Le score maximum étant MATE
 *
 *   Dans le cas d'un score provenant des tables Syzygy, on ne peut pas savoir si c'est un mat.
 *   On a toujours DTM=0 (Distance To Mat).
 *   C'est pourquoi, comme on veut toujours avoir un score indépendant de la profondeur de recherche,
 *   on prend une valeur référence différente et inférieure : TBWIN
 *   On ne prend pas une référence supérieure, car MATE est le score maximum que l'on peut avoir.
 *
 *   Comme la table de transposition sert à la fois pour les scores "normaux" et ceux provenant
 *   des tables Syzygy, on prend comme référence TBWIN.
 *
 */

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

//=========================================================

#ifdef HOME
const std::string Home = HOME;
#else
const std::string Home = "./";
#endif

#ifdef VERSION
const std::string Version = VERSION;
#else
const std::string Version = "2";
#endif

//=========================================================

extern void printlog(const std::string& message);
extern std::vector<std::string> split(const std::string& s, char delimiter);

//======================================
//! \brief Ecriture en binaire
//--------------------------------------
template<class T>
void binary_print(T code, const std::string& message)
{
    std::string binary = std::bitset<8*sizeof(T)>(code).to_string(); //to binary
    std::cout << message << " : " << binary<< std::endl;
}


#endif // DEFINES_H
