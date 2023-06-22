#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

//  Classe définissant la table de transposition
//  Voir le site de Moreland
//  Code provenant du code Sungorus (merci à lui)

enum HASH_CODE { HASH_NONE=0, HASH_ALPHA=1, HASH_BETA=2, HASH_EXACT=4 };

class TranspositionTable;

#include "defines.h"
#include "types.h"

/* Stockage SMP :
 --------------------------------
  U64   hash;    // 64 bits
  MOVE  move;    // 32 bits  (packing = 24 bits)  >> contient "hash_code"
  I16   score;   // 16 bits  0-65535
  U08   depth;   //  8 bits
  U08   date;    //  8 bits

  flag move score depth date
    3   24   16     8     8
             FFFF   FF    FF

 ATTENTION : pour stocker HASH_EXACT=4, il faut 3 bits
*/

struct HashEntry {
    U64   hash;    // 64 bits
    MOVE  move;    // 32 bits  (packing = 24 bits)  >> contient "hash_code"
    I16   score;   // 16 bits
    U08   depth;   //  8 bits
    U08   date;    //  8 bits
};

//----------------------------------------------------------
// Code provenant de Ethereal

static constexpr int EVAL_CACHE_KEY_SIZE = 16;
static constexpr int EVAL_CACHE_MASK     = 0xFFFF;
static constexpr int EVAL_CACHE_SIZE     = 1 << EVAL_CACHE_KEY_SIZE;

static constexpr int PAWN_CACHE_SIZE     = 64 * 1024;

using  EvalCacheEntry = U64;
struct PawnCacheEntry {
    U64      pawn_hash;
    Bitboard passed;
    Score    score;
};

//----------------------------------------------------------

/* Remarques
 *  1) le "hash_code" est mis dans "move" comme score
 *  2) dans Koivisto, la date (ou age) est mise dans move (8 bits) comme score
 */

// 134 217 728 = 2 ^ 27 = 128 Mo
// 0000 1000 0000 0000 0000 0000 0000 0000
//
//  8388608 entries of 16 for a total of 134217728
//
// tt_size = 8388608 = 100000000000000000000000 = 2 ^ 23
//
// tt_mask = tt_size - 4 = 0111 1111 1111 1111 1111 1100    : index multiple de 4
// tt_mask = tt_size - 2 = 0111 1111 1111 1111 1111 1110
// tt_mask = tt_size - 1 = 0111 1111 1111 1111 1111 1111

class TranspositionTable
{
private:
    int         tt_size;
    int         tt_mask;
    int         tt_date;
    int         tt_buckets;
    HashEntry*  tt_entries = nullptr;

//    int nbr_store[4];
//    int nbr_probe[4];

//    int hit;
//    int cut;

//    U64 pcachehit;
//    U64 pcachenohit;
//    U64 pcachestore;

    //-------------------------------------------
    EvalCacheEntry EvalCacheTable[EVAL_CACHE_SIZE];
    PawnCacheEntry PawnCacheTable[PAWN_CACHE_SIZE];

public:
    TranspositionTable();
    TranspositionTable(int MB);
    ~TranspositionTable();

    void set_size(int mbsize);
    void resize(int mbsize);

    void clear(void);
    void update_age(void);
    void store(U64 hash, MOVE move, int score, int flag, int depth, int ply);
    bool probe(U64 hash, int ply, MOVE &code, int& score, int &flag, int &depth);
    void stats();

    //! \brief Store terminal scores as distance from the current position to mate/TB
    int ScoreToTT (const int score, const int ply)
    {
        return score >=  TBWIN_IN_X ? score + ply
               : score <= -TBWIN_IN_X ? score - ply
                                        : score;
    }

    //! \brief Add the distance from root to terminal scores get the total distance to mate/TB
    int ScoreFromTT (const int score, const int ply)
    {
        return score >=  TBWIN_IN_X ? score - ply
               : score <= -TBWIN_IN_X ? score + ply
                                        : score;
    }

    //------------------------------------------------
    void store_evaluation(U64 hash, int eval);
    bool probe_evaluation(U64 hash, Color color, int& eval) const;
    void clear_eval_table(void);

    bool probe_pawn_cache(U64 hash, Score &score);
    void store_pawn_cache(U64 hash, Score score);
    void clear_pawn_table(void);
};

extern TranspositionTable Transtable;



#endif // TRANSPOSITIONTABLE_H
