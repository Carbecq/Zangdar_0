#include <cassert>
#include "TranspositionTable.h"
#include "defines.h"
#include <iostream>
#include <cstring>
#include "Move.h"


// Code inspiré de Sungorus
// Idées provenant de Bruce Moreland


#include <cassert>
#include "TranspositionTable.h"
#include "defines.h"


//========================================================
//! \brief  Constructeur avec argument
//--------------------------------------------------------
TranspositionTable::TranspositionTable(int MB) :
    pawn_size(PAWN_HASH_SIZE*1024),
    pawn_mask(PAWN_HASH_SIZE*1024 - 1)
{
#if defined DEBUG_LOG
    char message[100];
    sprintf(message, "TranspositionTable::constructeur MB : %d ", MB);
    printlog(message);
#endif

    tt_entries = nullptr;
    tt_size    = 0;
    tt_date    = 0;

    init_size(MB);
}

//========================================================
//! \brief  Destructeur
//--------------------------------------------------------
TranspositionTable::~TranspositionTable()
{
    delete [] tt_entries;
}

//========================================================
//! \brief  Initialisation de la table
//--------------------------------------------------------
void TranspositionTable::init_size(int mbsize)
{
#if defined DEBUG_LOG
    char message[1000];
    sprintf(message, "TranspositionTable::init_size : %d ", mbsize);
    printlog(message);
#endif

    int bytes    = mbsize * 1024 * 1024;
    int nbr_elem = bytes / sizeof(HashEntry);

    if (tt_entries != nullptr)
    {
        delete [] tt_entries;
        tt_entries = nullptr;
        tt_size = 0;
    }

    // size must be a power of 2!
    tt_size = 1;

    while (tt_size <= nbr_elem)
        tt_size *= 2;
    tt_size /= 2;

    assert(tt_size!=0 && (tt_size&(tt_size-1))==0); // power of 2

    // Blunder 8 : 2 buckets , age = 0 ou 1
    // Leorik    : 2 buckets
    // berserk   : 2 buckets

#if defined TT_SUNGORUS
    tt_buckets = 4;
#elif defined TT_LEORIK
    tt_buckets = 2;
#endif

    tt_mask    = tt_size - tt_buckets;
    tt_entries = new HashEntry[tt_size];

    clear();


#if defined DEBUG_LOG
    sprintf(message, "TranspositionTable init complete with %d entries of %lu bytes for a total of %lu bytes (%lu MB) \n",
            tt_size, sizeof(HashEntry), tt_size*sizeof(HashEntry), tt_size*sizeof(HashEntry)/1024/1024);
    printlog(message);
#endif
}

//========================================================
//! \brief  Allocation uniquement de la Hash Table
//--------------------------------------------------------
void TranspositionTable::set_hash_size(int mbsize)
{
#if defined DEBUG_LOG
    char message[100];
    sprintf(message, "TranspositionTable::set_hash_size : %d ", mbsize);
    printlog(message);
#endif

    delete [] tt_entries;
    tt_entries = nullptr;
    tt_size = 0;
    init_size(mbsize);
}

//========================================================
//! \brief  Remise à zéro de la table de transposition
//--------------------------------------------------------
void TranspositionTable::clear(void)
{
#if defined DEBUG_LOG
    char message[100];
    sprintf(message, "TranspositionTable::clear");
    printlog(message);
#endif

    tt_date = 0;

    std::memset(tt_entries, 0, sizeof(HashEntry) * tt_size);
    std::memset(pawn_entries, 0, sizeof(PawnHashEntry) * pawn_size);
}

//========================================================
//! \brief  Mise à jour de l'age
//! tt_date = [0...255]
//--------------------------------------------------------
void TranspositionTable::update_age(void)
{
    tt_date = (tt_date + 1) & 255;
}

//========================================================
//! \brief  Ecriture dans la hashtable d'une nouvelle donnée
//--------------------------------------------------------
void TranspositionTable::store(U64 hash, MOVE move, Score score, Score eval, int flag, int depth, int ply)
{
    /*
     *   https://www.talkchess.com/forum3/viewtopic.php?f=7&t=59047&sid=a772c728def68198d66e038c6905f820&start=10
     *   https://www.talkchess.com/forum3/viewtopic.php?f=7&t=76499
     *  https://github.com/vshcherbyna/igel/blob/master/src/tt.cpp
     *  https://www.talkchess.com/forum3/viewtopic.php?f=7&t=71994
     *  https://www.talkchess.com/forum3/viewtopic.php?f=7&t=71994&sid=a788b3887052ee53e04868f27afbaaa2&start=10
     *  https://www.talkchess.com/forum3/viewtopic.php?t=64604
     *  https://www.open-chess.org/viewtopic.php?f=5&t=3106
     *  https://www.talkchess.com/forum3/viewtopic.php?t=60589
     *  https://www.talkchess.com/forum3/viewtopic.php?t=60056
     *  https://home.hccnet.nl/h.g.muller/max-src2.html
     *  https://www.talkchess.com/forum3/viewtopic.php?f=7&t=57603&sid=4bdb5eafa3b7b3f4f6f321edcff1f89d&start=10
     *
     */

    // extract the 32-bit key from the 64-bit zobrist hash
    U32 key32 = hash >> 32;

#if defined TT_SUNGORUS

    HashEntry *entry=nullptr, *replace=nullptr;
    int oldest, age;

    score = ScoreToTT(score, ply);

    replace = nullptr;
    oldest  = -1;
    entry   = tt_entries + (hash & tt_mask);

    for (int i = 0; i < tt_buckets; i++)
    {
        if (entry->hash == key32)
        {
            if (!move)
                move = entry->move;
            replace = entry;
            break;
        }

        age = ((tt_date - entry->date) & 255) * 256 + 255 - entry->depth;
        if (age > oldest)
        {
            oldest  = age;
            replace = entry;
        }
        entry++;
    }

    replace->hash  = key32;
    replace->move  = move;
    replace->score = score;
    replace->eval  = eval;
    replace->depth = depth;
    replace->date  = tt_date;
    replace->flag  = flag;


#endif
}

//========================================================
//! \brief  Recherche d'une donnée
//! \param[in]  hash    code hash de la position recherchée
//! \param{out] code    coup trouvé
//! \param{out] score   score de ce coup
//! \param{in]  alpha
//! \param{in]  beta
//! \param{in]  depth
//! \param{in]  ply
//--------------------------------------------------------
bool TranspositionTable::probe(U64 hash, int ply, MOVE& move, Score& score, Score& eval, int &flag, int& depth)
{
    move  = Move::MOVE_NONE;
    score = NOSCORE;
    eval  = NOSCORE;
    flag  = 0;

    // extract the 32-bit key from the 64-bit zobrist hash
    U32 key32 = hash >> 32;

#if defined TT_SUNGORUS

    HashEntry* entry = tt_entries + (hash & tt_mask);

    for (int i = 0; i < tt_buckets; i++)
    {
        if (entry->hash == key32)
        {
            entry->date = tt_date;

            move  = entry->move;
            flag  = entry->flag;
            depth = entry->depth;
            score = ScoreFromTT(entry->score, ply);
            eval  = entry->eval;

            return true;
        }
        entry++;
    }


#endif

    return false;
}

//===============================================================
//! \brief  Recherche d'une donnée dans la table des pions
//! \param[in]  hash    code hash des pions
//! \param{out] score   score de cette position
//! \param[out] passed  bitboard des pions passés
//---------------------------------------------------------------
bool TranspositionTable::probe_pawn_table(U64 hash, Score &eval, Bitboard& passed)
{
    PawnHashEntry* entry = pawn_entries + (hash & pawn_mask);

    if (entry->hash == hash)
    {
        eval   = entry->eval;
        passed = entry->passed;
        return true;
    }

    return false;
}

//=============================================================
//! \brief Stocke une évaluation dans la table des pions
//!
//! \param[in]  hash    hash des pions
//! \param[in]  score   évaluation
//! \param[in]  passed  bitboard des pions passés
//-------------------------------------------------------------
void TranspositionTable::store_pawn_table(U64 hash, Score eval, Bitboard passed)
{
    PawnHashEntry* entry = pawn_entries + (hash & pawn_mask);

    entry->hash   = hash;
    entry->eval   = eval;
    entry->passed = passed;
}


void TranspositionTable::stats()
{
    std::cout << "TT size = " << tt_size << std::endl;
//    for (int i=0; i<tt_buckets; i++)
//    {
//        std::cout << "store[" << i+1 << "] = " << nbr_store[i] << std::endl;
//    }
//    for (int i=0; i<tt_buckets; i++)
//    {
//        std::cout << "probe[" << i+1 << "] = " << nbr_probe[i] << std::endl;
//    }

//    std::cout << "PawnCacheSize = " << PAWN_CACHE_SIZE << std::endl;
//    std::cout << "store  = " << pcachestore << std::endl;
//    std::cout << "hit    = " << pcachehit << std::endl;
//    std::cout << "no hit = " << pcachenohit << std::endl;
//    std::cout << "usage  = " << (double)pcachenohit / (double)pcachehit << std::endl;
}

//=======================================================================
//! \brief Estimation de l'utilisation de la table de transposition
//! On regarde combien d'entrées contiennent une valeur récente.
//! La valeur retournée va de 0 à 1000 (1 = 0.1 %)
//-----------------------------------------------------------------------
int TranspositionTable::hash_full()
{
    int used = 0;

    for (int i = 0; i < tt_buckets*1000; i++)
        if (   tt_entries[i].move != Move::MOVE_NONE
            && tt_entries[i].date == tt_date)
            used++;

    return used/tt_buckets;
}

