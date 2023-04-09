#include <cassert>
#include "TranspositionTable.h"
#include "defines.h"
#include <bitset>
#include <iostream>
#include <cstring>
#include "move.h"

// Code inspiré de Sungorus
// Idées provenant de Bruce Moreland


#include <cassert>
#include "TranspositionTable.h"
#include "defines.h"


//========================================================
//! \brief  Mise à jour de l'age
//--------------------------------------------------------
void TranspositionTable::update_age(void)
{
    tt_date = (tt_date + 1) & 255;
}

//========================================================
//! \brief  Constructeur
//--------------------------------------------------------
TranspositionTable::TranspositionTable()
{
    init(128);

    U32 code = tt_size*sizeof(HashEntry);
    std::string binary = std::bitset<32>(code).to_string(); //to binary
    std::cout<< " taille totale " << binary<<"\n";

    printf("TranspositionTable init complete with %d entries of %lu for a total of %lu \n",
           tt_size, sizeof(HashEntry), tt_size*sizeof(HashEntry));

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
void TranspositionTable::init(int mbsize)
{
    int bytes    = mbsize * 1024 * 1024;
    int nbr_elem = bytes / sizeof(HashEntry);

    // size must be a power of 2!
    tt_size = 1;

    while (tt_size <= nbr_elem)
        tt_size *= 2;
    tt_size /= 2;

    assert(tt_size!=0 && (tt_size&(tt_size-1))==0); // power of 2


    tt_buckets = 4;
    tt_mask = tt_size - tt_buckets;

    delete [] tt_entries;
    tt_entries = new HashEntry[tt_size];
    clear();

    tt_date = 0;
}

//========================================================
//! \brief  Remise à zéro de la table
//--------------------------------------------------------
void TranspositionTable::clear(void)
{
    //    std::memset(tt_entries, 0, sizeof(HashEntry) * tt_size);
    tt_date = 0;

    for (HashEntry* entry = tt_entries; entry < tt_entries + tt_size; entry++)
    {
        entry->hash = 0;
        entry->date = 0;
        entry->move = 0;
        entry->score = 0;
        entry->depth = 0;
    }

    for (int i=0; i<4; i++)
    {
        nbr_probe[i] = 0;
        nbr_store[i] = 0;
    }
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

        entry++;
    }

  //  nbr_store[k]++;

    replace->hash = hash;
    replace->date = tt_date;
    replace->move = Move::set_flag(move, flag);
    replace->score = score;
    replace->depth = depth;
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
        if (entry->hash == hash)
        {
            entry->date = tt_date;

            move = Move::move(entry->move);
            flag = Move::flag(entry->move);

            if (entry->depth >= depth)
            {
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
