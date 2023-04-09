#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

//  Classe définissant la table de transposition
//  Voir le site de Moreland
//  Code provenant du code Sungorus (merci à lui)

enum HASH_CODE { HASH_NONE=0, HASH_ALPHA=1, HASH_BETA=2, HASH_EXACT=4 };

class TranspositionTable;

#include "defines.h"


struct HashEntry {
  U64   hash;    // 64 bits
  MOVE  move;   // 32 bits  (packing = 24 bits)  >> contient "hash_code"
  I16   score;  // 16 bits
  U08   depth;  //  8 bits
  U08   date;    //  8 bits
};


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

    int nbr_store[4];
    int nbr_probe[4];

public:
    TranspositionTable();
    ~TranspositionTable();

    void init(int mbsize);
    void clear(void);
    void update_age(void);
    void store(U64 hash, MOVE move, int score, int flag, int depth, int ply);
    bool probe(U64 hash, MOVE &code, int& score, int &flag, int alpha, int beta, int depth, int ply);
    void stats();

};

#endif // TRANSPOSITIONTABLE_H
