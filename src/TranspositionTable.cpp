#include <cassert>
#include "TranspositionTable.h"
#include "defines.h"
#include <bitset>
#include <iostream>
#include <cstring>
#include "Move.h"


// Code inspiré de Sungorus
// Idées provenant de Bruce Moreland


#include <cassert>
#include "TranspositionTable.h"
#include "defines.h"

//========================================================
//! \brief  Constructeur défaut
//--------------------------------------------------------
TranspositionTable::TranspositionTable()
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "TranspositionTable::constructeur : %d ", DEFAULT_HASH_SIZE);
    printlog(message);
#endif
    tt_entries = nullptr;
    tt_size    = 0;
    tt_date    = 0;

    set_size(DEFAULT_HASH_SIZE);
}

//========================================================
//! \brief  Constructeur avec argument
//--------------------------------------------------------
TranspositionTable::TranspositionTable(int MB)
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "TranspositionTable::constructeur MB : %d ", MB);
    printlog(message);
#endif
    tt_entries = nullptr;
    tt_size    = 0;
    tt_date    = 0;

    set_size(MB);
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
void TranspositionTable::set_size(int mbsize)
{
#ifdef DEBUG_LOG
    char message[1000];
    sprintf(message, "TranspositionTable::set_size : %d ", mbsize);
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

    tt_buckets = 4;
    //    mask = tt_size - 1;    // Koivisto, Fruit21 (clustersize=4), Berserk (bucketsize=2)
    //    mask = size - 2;
    //    mask = size - 4;        // Sungorus, Rodent
    tt_mask = tt_size - tt_buckets;

    tt_entries = new HashEntry[tt_size];
    clear();

#ifdef DEBUG_LOG
    sprintf(message, "TranspositionTable init complete with %d entries of %lu bytes for a total of %lu bytes (%lu MB) \n",
            tt_size, sizeof(HashEntry), tt_size*sizeof(HashEntry), tt_size*sizeof(HashEntry)/1024/1024);
    printlog(message);
#endif
}

//========================================================
//! \brief  Re-Initialisation de la table
//--------------------------------------------------------
void TranspositionTable::resize(int mbsize)
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "TranspositionTable::resize : %d ", mbsize);
    printlog(message);
#endif

    delete [] tt_entries;
    tt_entries = nullptr;
    tt_size = 0;
    set_size(mbsize);
}

#ifdef TT_XOR
//void VerifyEntrySMP(int k, HashEntry *entry)
//{
    //    int e_flag = Move::flag(entry->move);
    //    int e_move = Move::move(entry->move);

    //    U64 smp_data = FOLD_DATA(entry->score, entry->depth, e_flag, e_move);
    //    U64 smp_key  = entry->hash ^ smp_data;

    //    if (smp_data != entry->smp_data) { printf("data error %d", k); exit(1);}
    //    if (smp_key != entry->smp_key) { printf("smp_key error %d", k); exit(1);}

    //    int move = EXTRACT_MOVE(smp_data);
    //    int flag = EXTRACT_FLAGS(smp_data);
    //    int score = EXTRACT_SCORE(smp_data);
    //    int depth = EXTRACT_DEPTH(smp_data);

    //    if (move != e_move) { printf("move error %d", k); exit(1);}
    //    if (score != entry->score) { printf("score error %d", k); exit(1);}
    //    if (depth != entry->depth) { printf("depth error %d", k); exit(1);}
    //    if (flag != e_flag) { printf("flags error %d (smp_flag=%d entry_flag=%d) \n", k, flag, e_flag);
    //        printf("%s \n", Move::name(e_move).c_str());
    //        printf("%s \n", Move::name(move).c_str());
    //        exit(1);
    //    }
//}
#endif

//========================================================
//! \brief  Remise à zéro de la table
//--------------------------------------------------------
void TranspositionTable::clear(void)
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "TranspositionTable::clear");
    printlog(message);
#endif

    //    std::memset(tt_entries, 0, sizeof(HashEntry) * tt_size);
    tt_date = 0;

    for (HashEntry* entry = tt_entries; entry < tt_entries + tt_size; entry++)
    {
#ifdef TT_XOR
        entry->smp_key = 0ULL;
        entry->smp_data = 0ULL;
#else
        entry->hash = 0;
        entry->date = 0;
        entry->move = 0;
        entry->score = 0;
        entry->depth = 0;
#endif
    }

    for (int i=0; i<4; i++)
    {
        nbr_probe[i] = 0;
        nbr_store[i] = 0;
    }
}

//========================================================
//! \brief  Mise à jour de l'age
//--------------------------------------------------------
void TranspositionTable::update_age(void)
{
    tt_date = (tt_date + 1) & 255;
}


//========================================================
//! \brief  Ecriture dans la hashtable d'une nouvelle donnée
//--------------------------------------------------------
void TranspositionTable::store(U64 hash, MOVE move, int score, int flag, int depth, int ply)
{
    HashEntry *entry=nullptr, *replace=nullptr;
    int oldest, age;
    //         int k=0;

    if (score < -MAX_EVAL)
        score -= ply;
    else if (score > MAX_EVAL)
        score += ply;

    replace = nullptr;
    oldest  = -1;
    entry   = tt_entries + (hash & tt_mask);

    for (int i = 0; i < tt_buckets; i++)
    {
#ifdef TT_XOR
        if (entry->smp_key == (hash ^ entry->smp_data))
        {
            if (!move)
                move = EXTRACT_MOVE(entry->smp_data);
            replace = entry;
            //     k = i;
            break;
        }

        age = ((tt_date - EXTRACT_DATE(entry->smp_data)) & 255) * 256 + 255 - EXTRACT_DEPTH(entry->smp_data);
        if (age > oldest)
        {
            oldest  = age;
            replace = entry;
            //          k = i;
        }
#else
        if (entry->hash == hash)
        {
            if (!move)
                move = Move::move(entry->move);
            replace = entry;
            //     k = i;
            break;
        }

        age = ((tt_date - entry->date) & 255) * 256 + 255 - entry->depth;
        if (age > oldest)
        {
            oldest  = age;
            replace = entry;
            //          k = i;
        }

        //      printf("tt_date=%d entry.date=%d entry.depth=%d >>> age=%d k=%d \n", tt_date, entry->date, entry->depth, age, k);
#endif
        entry++;
    }



    //  nbr_store[k]++;

#ifdef TT_XOR
    U64 smp_data = FOLD_DATA(tt_date, depth, score, move, flag);
    //   U64 smp_key  = hash ^ smp_data;

    replace->smp_data = smp_data;
    replace->smp_key  = hash ^ smp_data;

    //   VerifyEntrySMP(1, replace);
#else
    replace->hash  = hash;
    replace->date  = tt_date;
    replace->move  = Move::set_flag(move, flag);
    replace->score = score;
    replace->depth = depth;
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
bool TranspositionTable::probe(U64 hash, MOVE& move, int& score, int &flag, int alpha, int beta, int depth, int ply)
{
    move  = 0;
    score = INVALID;
    flag  = HASH_NONE;

    HashEntry* entry = tt_entries + (hash & tt_mask);

    for (int i = 0; i < tt_buckets; i++)
    {
#ifdef TT_XOR
        if (entry->smp_key == (hash ^ entry->smp_data))
        {
            //            U64 test_key = entry->hash ^ entry->smp_data;
            //            if(test_key != entry->smp_key) printf("Error test_key\n");

            //            VerifyEntrySMP(2, entry);
            int smp_depth = EXTRACT_DEPTH(entry->smp_data);
            int smp_score = EXTRACT_SCORE(entry->smp_data);
            int smp_move  = EXTRACT_MOVE(entry->smp_data);
            int smp_flag  = EXTRACT_FLAG(entry->smp_data);

            entry->smp_data = FOLD_DATA(tt_date, smp_depth, smp_score, smp_move, smp_flag);
            //           entry->date = tt_date;

            move = smp_move; //Move::move(entry->move);
            flag = smp_flag; //Move::flag(entry->move);

            if (smp_depth >= depth)
            {
                //        hit++;

                score = smp_score;

                if (score < -MAX_EVAL)
                    score += ply;
                else if (score > MAX_EVAL)
                    score -= ply;

                if ((smp_flag & HASH_ALPHA && score <= alpha) ||
                        (smp_flag & HASH_BETA && score >= beta))
                {
                    //                    nbr_probe[i]++;
                    return true;
                }
            }
            break;
        }
#else
        if (entry->hash == hash)
        {
            entry->date = tt_date;

            move = Move::move(entry->move);
            flag = Move::flag(entry->move);

            if (entry->depth >= depth)
            {
                //        hit++;

                score = entry->score;

                if (score < -MAX_EVAL)
                    score += ply;
                else if (score > MAX_EVAL)
                    score -= ply;

                if ((flag & HASH_ALPHA && score <= alpha) ||
                        (flag & HASH_BETA && score >= beta))
                {
                    //                    nbr_probe[i]++;
                    return true;
                }
            }
            break;
        }
#endif
        entry++;
    }

    return false;
}


void TranspositionTable::stats()
{
    std::cout << "size=" << tt_size << std::endl;
    for (int i=0; i<tt_buckets; i++)
    {
        std::cout << "store[" << i+1 << "] = " << nbr_store[i] << std::endl;
    }
    for (int i=0; i<tt_buckets; i++)
    {
        std::cout << "probe[" << i+1 << "] = " << nbr_probe[i] << std::endl;
    }

}


