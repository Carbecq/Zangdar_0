#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

//  Classe définissant la table de transposition
//  Voir le site de Moreland
//  Code provenant du code Sungorus (merci à lui)

static constexpr int BOUND_NONE  = 0;
static constexpr int BOUND_LOWER = 1;   // hash_beta    Cut-nodes (Knuth's Type 2), otherwise known as fail-high nodes, are nodes in which a beta-cutoff was performed.
static constexpr int BOUND_UPPER = 2;   // hash_alpha   All-nodes (Knuth's Type 3), otherwise known as fail-low nodes, are nodes in which no move's score exceeded alpha
static constexpr int BOUND_EXACT = 4;   //              PV-node   (Knuth's Type 1) are nodes that have a score that ends up being inside the window

class TranspositionTable;

#include "defines.h"

struct HashEntry {
    U32   hash;     // 32 bits
    MOVE  move;     // 32 bits  (packing = 24 bits)
    I16   score;    // 16 bits
    I16   eval;     // 16 bits
    U08   depth;    //  8 bits
    U08   date;     //  8 bits
    U08   flag;     //  8 bits  (3 bits nécessaires seulement)
};

//----------------------------------------------------------
// Code provenant de Ethereal

struct PawnHashEntry {
    U64      hash;      // 64 bits
    Bitboard passed;    // 64 bits
    Score    eval;      // 32 bits
};

//----------------------------------------------------------

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

    // Table de transposition pour les pions
    PawnHashEntry   pawn_entries[PAWN_HASH_SIZE*1024];
    int             pawn_size;
    int             pawn_mask;

public:
    TranspositionTable(int MB);
    ~TranspositionTable();

    void init_size(int mbsize);
    void set_hash_size(int mbsize);

    void clear(void);
    void update_age(void);
    void store(U64 hash, MOVE move, Score score, Score eval, int flag, int depth, int ply);
    bool probe(U64 hash, int ply, MOVE &code, Score &score, Score &eval, int &flag, int &depth);
    void stats();
    int  hash_full();

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

    bool probe_pawn_table(U64 hash, Score &eval);
    void store_pawn_table(U64 hash, Score eval);
};

extern TranspositionTable transpositionTable;



#endif // TRANSPOSITIONTABLE_H
