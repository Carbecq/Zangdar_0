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

    tt_buckets = 4;
    tt_mask = tt_size - tt_buckets;

    tt_entries = new HashEntry[tt_size];
    clear();
    clear_pawn_table();
    clear_eval_table();


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
//! \brief  Remise à zéro de la table de transposition
//--------------------------------------------------------
void TranspositionTable::clear(void)
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "TranspositionTable::clear");
    printlog(message);
#endif

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

}

//========================================================
//! \brief  Remise à zéro de la table des pions
//--------------------------------------------------------
void TranspositionTable::clear_pawn_table(void)
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "TranspositionTable::clear_pawn_table");
    printlog(message);
#endif

    std::memset(PawnCacheTable, 0, sizeof(PawnCacheEntry) * PAWN_CACHE_SIZE);

}

//========================================================
//! \brief  Remise à zéro de la table des évaluations
//--------------------------------------------------------
void TranspositionTable::clear_eval_table(void)
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "TranspositionTable::clear_eval_table");
    printlog(message);
#endif

    std::memset(EvalCacheTable, 0, sizeof(U64) * EVAL_CACHE_SIZE);

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

    score = ScoreToTT(score, ply);

    replace = nullptr;
    oldest  = -1;
    entry   = tt_entries + (hash & tt_mask);

    for (int i = 0; i < tt_buckets; i++)
    {
        if (entry->hash == hash)
        {
            if (!move)
                move = Move::get_move(entry->move);
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


    replace->hash  = hash;
    replace->date  = tt_date;
    replace->move  = Move::get_code_move(move, flag);
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
bool TranspositionTable::probe(U64 hash, int ply, MOVE& move, int& score, int &flag, int& depth)
{
    move  = Move::MOVE_NONE;
    score = NOSCORE;
    flag  = BOUND_NONE;

    HashEntry* entry = tt_entries + (hash & tt_mask);

    for (int i = 0; i < tt_buckets; i++)
    {
        if (entry->hash == hash)
        {
            entry->date = tt_date;

            move  = Move::get_move(entry->move);
            flag  = Move::get_code(entry->move);
            depth = entry->depth;
            score = ScoreFromTT(entry->score, ply);

            return true;
        }
        entry++;
    }

    return false;
}

//================================================================
//! \brief  Stockage de l'évaluation dans le cache
//! \param[in] hash     clef Zobrist de la position
//! \param[in] eval     évaluation
//----------------------------------------------------------------
void TranspositionTable::store_evaluation(U64 hash, int eval)
{
    // Code repris d'Ethereal
    // On combine en 1 seule valeur la clef et l'évaluation
    // Voir aussi comment on fait dans l'évaluation S(mg, eg)

    // On met à 0 les 16 bits (0-15) du hash
    // pour y stocker l'évaluation
    EvalCacheTable[hash & EVAL_CACHE_MASK]
        = (hash & ~0xFFFF) | (uint16_t)((int16_t)eval);
}

//================================================================
//! \brief  Récupération de l'évaluation dans le cache
//! \param[in]  hash    clef Zobrist de la position
//! \param[in]  color   camp au trait
//! \param[out] eval    évaluation
//----------------------------------------------------------------
bool TranspositionTable::probe_evaluation(U64 hash, Color color, int& eval) const
{
    EvalCacheEntry eve;
    U64       key;

    eve = EvalCacheTable[hash & EVAL_CACHE_MASK];
    key = (eve & ~0xFFFF) | (hash & 0xFFFF);

    eval = (int16_t)((uint16_t)(eve & 0xFFFF));
    eval = (color == WHITE) ? eval : -eval;
    return hash == key;
}

//===============================================================
//! \brief  Recherche d'une donnée dans la table des pions
//! \param[in]  hash    code hash des pions
//! \param{out] score   score de cette position
//---------------------------------------------------------------
bool TranspositionTable::probe_pawn_cache(U64 hash, Score &score)
{
    PawnCacheEntry* entry = &PawnCacheTable[hash % PAWN_CACHE_SIZE];

    if (entry->pawn_hash == hash)
    {
        score = entry->score;
        return true;
    }
    return false;
}

//=============================================================
//! \brief Stocke une évaluation dans la table des pions
//!
//! \param[in]  hash    hash des pions
//! \param[in]  score   évaluation
//-------------------------------------------------------------
void TranspositionTable::store_pawn_cache(U64 hash, Score score)
{
    PawnCacheEntry* entry = &PawnCacheTable[hash % PAWN_CACHE_SIZE];

    entry->pawn_hash = hash;
    entry->score     = score;
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


