#ifndef HASHTABLE_H
#define HASHTABLE_H

//  Classe définissant la table de transposition
//  Voir le site de Moreland

class HashTable;

#include "defines.h"

struct HashEntry {
    U64     hash;   // hash de la position
    U32     code;   // code du coup ayant amené à la position
    int     score;  //
    int     depth;
    U32     flags;

    HashEntry() : hash(0), code(0), score(0), depth(0), flags(0) {}
} ;

const int HashSize = 128 << 20; // 128 MB

class HashTable
{
private:
    HashEntry*  entries;
    int         numEntries;
    int         newWrite;
    int         overWrite;
//    int         hit;

public:
    HashTable();
    ~HashTable();

//    int cut;

    void clear();
    void store(U64 hash, U32 code, int score, int flags, int depth, int ply);

    // Used to print stats
    int size() const {
        return  numEntries;
    };

    U32 code_at(int i) const {
        return entries[i].code;
    };
    U64 hash_at(int i) const {
        return entries[i].hash;
    };

    bool probe(U64 hash, U32& code, int& score, int alpha, int beta, int depth, int ply);


};

#endif // HASHTABLE_H
