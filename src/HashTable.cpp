#include <cassert>
#include "HashTable.h"
#include "defines.h"
#include "Position.h"

// Code provenant de Bruce Moreland
// voir Purple Haze, Stockfish, Mediocre

//========================================================
//! \brief  Constructeur
//--------------------------------------------------------
HashTable::HashTable()
{
    newWrite    = 0;
    overWrite   = 0;
//    hit         = 0;
    numEntries  = HashSize / sizeof(HashEntry),
    entries     = new HashEntry[numEntries];

    printf("HashTable init complete with %d entries for %d Mo\n", numEntries, HashSize/1024/1024);
}

//========================================================
//! \brief  Destructeur
//--------------------------------------------------------
HashTable::~HashTable()
{
    delete [] entries;
    entries = nullptr;
    numEntries = 0;
}

//========================================================
//! \brief  Remise à zéro
//--------------------------------------------------------
void HashTable::clear()
{
    std::fill(entries, entries + numEntries, HashEntry());
    newWrite    = 0;
    overWrite   = 0;
//    hit         = 0;
}

//========================================================
//! \brief  Ecriture dans la hashtable d'une nouvelle donnée
//--------------------------------------------------------
void HashTable::store(U64 hash, U32 code, int score, int flags, int depth, int ply)
{
    int index = hash % numEntries;  //TODO autre manière de trouver l'index ?

    //
    assert(index >= 0 && index <= numEntries - 1);

    assert(depth >= 1 && depth < MAX_PLY);                  //TODO depth >= 1 ???
    assert(flags >= HASH_ALPHA && flags <= HASH_EXACT);
    assert(score >= -MAX_SCORE && score <= MAX_SCORE);
    assert(ply >= 0 && ply < MAX_PLY);

    //TODO voir comment on écrase , sous quelle condition
    if( entries[index].hash == 0)
    {
        newWrite++;
    } else {
        overWrite++;
    }

    // On stoke le score de façon indépendante de la profondeur de recherche (ply)
    //  si score = -9993 au ply 7 -10000 +7
    //  -> on retourne (-9993)
    //  -> à l'itération précédente : alpha = +9993, et on stocke 9993+6 = 9999


    if(score > IS_MATE)
        score += ply;
    else if(score < -IS_MATE)
        score -= ply;

    entries[index].code  = code;
    entries[index].hash  = hash;
    entries[index].flags = flags;
    entries[index].score = score;
    entries[index].depth = depth;
}

//========================================================
//! \brief  Lecture d'une donnée
//--------------------------------------------------------
bool HashTable::probe(U64 hash, U32& code, int& score, int alpha, int beta, int depth, int ply)
{
    int index = hash % numEntries;

    assert(index >= 0 && index <= numEntries - 1);

    assert(depth >=1 && depth < MAX_PLY);
    assert(alpha < beta);
    assert(alpha >= -MAX_SCORE && alpha <= MAX_SCORE);
    assert(beta  >= -MAX_SCORE && beta  <= MAX_SCORE);
    assert(ply >= 0 && ply < MAX_PLY);

    if( entries[index].hash == hash )
    {
        code = entries[index].code;
        if(entries[index].depth >= depth)   //TODO ?
        {
 //           hit++;

            assert(entries[index].depth >= 1 && entries[index].depth < MAX_PLY);
            assert(entries[index].flags >= HASH_ALPHA && entries[index].flags <= HASH_EXACT);

            score = entries[index].score;

            // On récupère le score indépendant de la profondeur
            // à laquelle le coup a été trouvé (et stocké)

            // plus tard, on fait un probe avec la même profondeur (depth)
            // on trouve un coup avec un score identique 9999
            // si on est à ply = 3 -> score devient 9996 : mat en 4 demi-coups

            /* ply = 0  Txh7+
             *       1        Rxh7
             *       2  Dh5+
             *       3        Rg8
             *       4  Df7+
             *       5        Rh8
             *       6  Dxg7+
             *       7        mat
             *
             */

            if(score > IS_MATE)
                score -= ply;
            else if (score < -IS_MATE)
                score += ply;

            assert(score >= -MAX_SCORE && score <= MAX_SCORE);

            switch(entries[index].flags)
            {
                case HASH_ALPHA:
                    if(score <= alpha)
                    {
                        score = alpha;
                        return true;    // return alpha;
                    }
                    break;
                case HASH_BETA:
                    if(score >= beta)
                    {
                        score = beta;
                        return true;    // return beta;
                    }
                    break;
                case HASH_EXACT:
                    return true;        // return score; ou entries[index]
                    break;
                default:
                    assert(false);
                    break;
            }
        }
    }

    return false;
}











