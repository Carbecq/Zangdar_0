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

  //  assert(depth >= 1 && depth < MAX_PLY);                  //TODO depth >= 1 ???
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
//! \brief  Recherche d'une donnée
//! \param[in]  hash    code hash de la position recherchée
//! \param{out] code    coup trouvé
//! \param{out] score   score de ce coup
//! \param{in]  alpha
//! \param{in]  beta
//! \param{in]  depth
//! \param{in]  ply
//--------------------------------------------------------
bool HashTable::probe(U64 hash, U32& code, int& score, int alpha, int beta, int depth, int ply)
{
    // commentaires de Blunder 5
    // le site de Moreland contient d'autres informations intéressantes

    // Voir le code Gerbil, bien que la notation ne soit pas aisée

    // Get the entry from the table, calculating an index by modulo-ing the hash of
    // the position by the size of the table.

    // note : une autre manière d'obtenir cet index est possible
    //TODO : à voir
    int index = hash % numEntries;

    assert(index >= 0 && index <= numEntries - 1);

 //   assert(depth >=1 && depth < MAX_PLY);
    assert(alpha < beta);
    assert(alpha >= -MAX_SCORE && alpha <= MAX_SCORE);
    assert(beta  >= -MAX_SCORE && beta  <= MAX_SCORE);
    assert(ply >= 0 && ply < MAX_PLY);

    // Since index collisions can occur, test if the hash of the entry at this index
    // actually matches the hash for the current position.
    if( entries[index].hash == hash )
    {
        // Even if we don't get a score we can use from the table, we can still
        // use the best move in this entry and put it first in our move ordering
        // scheme.
        code = entries[index].code;

        // To be able to get an accurate value from this entry, make sure the results of
        // this entry are from a search that is equal or greater than the current
        // depth of our search.
        if(entries[index].depth >= depth)
        {
 //           hit++;

    //        assert(entries[index].depth >= 1 && entries[index].depth < MAX_PLY);
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

            // If the score we get from the transposition table is a checkmate score, we need
            // to do a little extra work. This is because we store checkmates in the table using
            // their distance from the node they're found in, not their distance from the root.
            // So if we found a checkmate-in-8 in a node that was 5 plies from the root, we need
            // to store the score as a checkmate-in-3. Then, if we read the checkmate-in-3 from
            // the table in a node that's 4 plies from the root, we need to return the score as
            // checkmate-in-7.

            if(score > IS_MATE)
                score -= ply;
            else if (score < -IS_MATE)
                score += ply;

            assert(score >= -MAX_SCORE && score <= MAX_SCORE);

            switch(entries[index].flags)
            {
                // Dans les cas ALPHA ou BETA, mettre le score à alpha, ou beta, peur mener à
                // des instabilités

                case HASH_ALPHA:
                // If we have an alpha entry, and the entry's score is less than our
                // current alpha, then we know that our current alpha is the best score
                // we can get in this node, so we can stop searching and use alpha.
                    if(score <= alpha)
                    {
                        score = alpha;
                        return true;    // return alpha;
                    }
                    break;
                case HASH_BETA:
                // If we have a beta entry, and the entry's score is greater than our
                // current beta, then we have a beta-cutoff, since while
                // searching this node previously, we found a value greater than the current
                // beta. so we can stop searching and use beta.
                    if(score >= beta)
                    {
                        score = beta;
                        return true;    // return beta;
                    }
                    break;
                case HASH_EXACT:
                // If we have an exact entry, we can use the saved score.
                    return true;        // return score; ou entries[index]
                    break;
                default:
                    assert(false);
                    break;
            }
        } // depth OK
    } // hash OK

    return false;
}











