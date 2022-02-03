#include <cstring>
#include <iostream>
#include <fstream>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <locale>
#include "Search.h"
#include "Piece.h"
#include "Timer.h"
#include "Board.h"

// C'est ici que les Athéniens s'atteignirent ...
//  Routines en constante évolution, c'est le coeur du code.
//  Je n'ai rien inventé, mais repris des idées ici et là.

#include <string>
#include <iostream>
#include <sstream>

// https://www.chessprogramming.org/CPW-Engine_search
// https://fr.wikipedia.org/wiki/%C3%89lagage_alpha-b%C3%AAta
// https://www.hackerearth.com/blog/developers/minimax-algorithm-alpha-beta-pruning/
// https://stackoverflow.com/questions/19944529/how-exactly-to-use-history-heuristic-in-alpha-beta-minimax
// https://stackoverflow.com/questions/16500739/chess-high-branching-factor/16642804#16642804
// https://stackoverflow.com/questions/1110439/chess-optimizations

Search::Search()
{
#ifdef DEBUG_CLASS
    std::cout << "Search::Search" << std::endl;
#endif
    init();
}

Search::~Search()
{
#ifdef DEBUG_CLASS
    std::cout << "Search::~Search" << std::endl;
#endif
}

void Search::init()
{
#ifdef DEBUG_CLASS
    std::cout << "Search::init" << std::endl;
#endif

    Board::init();

    stopped             = false;
    nodes               = 0;
    limitCheckCount     = 0;
    logUci              = true;
    logSearch           = true;
    logTactics          = false;
}

//-----------------------------------------------------
//! \brief Remise à zéro
//-----------------------------------------------------
void Search::reset()
{
#ifdef DEBUG_CLASS
    std::cout << "Search::reset" << std::endl;
#endif
    Board::reset();
}

//-----------------------------------------------------
//! \brief Initialisation depuis une chaine FEN
//-----------------------------------------------------
void Search::init_fen(const std::string& fen)
{
#ifdef DEBUG_CLASS
    std::cout << "Search::init_fen" << std::endl;
#endif
    Board::init_fen(fen, logTactics);
}

//-----------------------------------------------------
//! \brief Initialisation au début d'une recherche
//!        dans le cadre d'une partie
//!     Ne pas confondre avec la fonction new_game,
//!     qui remet à 0 la hashtable.
//-----------------------------------------------------
void Search::new_search()
{
#ifdef DEBUG_CLASS
    std::cout << "Search::new_search" << std::endl;
#endif

    stopped         = false;
    limitCheckCount = 0;
    nodes           = 0;
    seldepth        = 0;

    for (int i=0; i<MAX_PLY; i++)
    {
        for (int j=0; j<MAX_PLY; j++)
        {
            pv[i][j] = "plop";
        }
        pv_length[i] = 0;
    }

    positions->ply          = 0;
    // hply n'est pas modifié, car on est toujours
    // dans la partie

    for (int i=0; i<2; i++) {
        for(int index = 0; index < 7; ++index) {
            for(int index2 = 0; index2 < BOARD_SIZE; ++index2) {
                searchHistory[i][index][index2] = 0;
            }
        }
    }

    for(int index = 0; index < 2; ++index) {
        for(int index2 = 0; index2 < MAX_PLY; ++index2) {
            searchKillers[index][index2] = 0;
        }
    }
}

//======================================================
//! \brief  Lancement d'une recherche
//!
//------------------------------------------------------
void Search::think()
{
#ifdef DEBUG_CLASS
    std::cout << "Search::think" << std::endl;
#endif

    new_search();
    timer.startIteration();

    int targetDepth = timer.getSearchDepth();
    std::string best_move;

    // iterative deepening
    for(int currentDepth = 1; currentDepth <= targetDepth; ++currentDepth )
    {
        int alpha = -MAX_SCORE;
        int beta  = MAX_SCORE;
        seldepth  = 0;

        int score = search_root(alpha, beta, currentDepth, best_move);

        if (stopped)
            break;

        // L'itération s'est terminée sans problème
        // On peut mettre à jour les infos UCI

        int elapsed = 0;
        bool shouldStop = timer.finishOnThisDepth(elapsed, nodes, 0);

        if (logUci)
            show_uci_result(currentDepth, seldepth,  score, elapsed);

        if (abs(score) > IS_MATE)
            break;

        // est-ce qu'on a le temps pour une nouvelle itération ?
        if (shouldStop == true)
            break;
    }

    if (logUci)
        show_uci_best(best_move);

    if (logSearch)
        timer.show_time();
}


//======================================================
//! \brief  Lancement d'une recherche
//!
//! Je fais une recherche en 2 temps
//! + d'abord la racine
//! + puis alpha-beta sur le reste
//!
//------------------------------------------------------
int Search::search_root(int alpha, int beta, int depth, std::string& best_move)
{
#ifdef DEBUG_CLASS
    std::cout << "Search::root" << std::endl;
#endif
    assert(beta > alpha);
    assert(depth >= 0);

     int legal = 0;
    int  temp_score = -MAX_SCORE;
    U32  best_code  = 0;
    int  old_alpha = alpha;

    // génération des coups
    gen_moves();

    for (int index = first_move[positions->ply]; index < first_move[positions->ply + 1]; ++index)
    {
        if (make_move(&moves[index]) == false)
            continue;
        legal++;
        unmake_move(&moves[index]);
    }

    if (legal == 0)
    {
        std::cout << "**********************************legal=0" << std::endl;
        return -MAX_SCORE;
    }

    std::string echec;
    std::string plus[2] = {"", "+"};
    bool do_PVS = false;

    nodes++;

    // Time-out ou arrêt
//    if (stopped || checkLimits())
//    {
//        stopped = true;
//        return 0;
//    }

    pv_length[positions->ply] = positions->ply;

    // Nullité par répétition ou règle des 50 coups
    if(positions->is_draw(history))
        return 0;

    // Si on est en échec, on cherche plus profondément
    bool in_check = is_in_check(positions->side_to_move);
    if (in_check == true)
        ++depth;

    int score       = -MAX_SCORE;
    U32 ht_move     = 0;

    // On utilise la hashtable seulement pour trier les coups
    hashtable.probe(positions->hash, ht_move, score, alpha, beta, depth, positions->ply);

    // Tri des coups de la PV line
    if (ht_move != 0)
    {
        pv_move(ht_move, positions->ply);
    }

 //   std::string some_move;

    for (int index = first_move[positions->ply]; index < first_move[positions->ply + 1]; ++index)
    {
        // prend le meilleur coup, basé sur le score defini dans la génération des coups
        PickNextMove(index);

        if (make_move(&moves[index]) == false)
            continue;

        if (logTactics)
            echec = plus[is_in_check(positions->side_to_move)];

        if (do_PVS)
        {
            // On peut essayer la PVS (Principal Variation Search)
            score = -alpha_beta(-alpha - 1, -alpha, depth - 1, true);

            // Est-ce que ça a marché ?
//            if (score > alpha)
            if (score > alpha && score < beta)  //TODO test à vérifier
            {
                // Raté, il faut refaire une recherche complète
                score = -alpha_beta(-beta, -alpha, depth - 1, true);
            }
        }
        else
        {
            score = -alpha_beta(-beta, -alpha, depth - 1, true);
        }

        unmake_move(&moves[index]);
  //      some_move = moves[index].show(output);

        if (stopped || checkLimits())
        {
            stopped = true;
            break;
        }

        // On a trouvé un bon coup
        if(score > temp_score)
        {
            temp_score = score;    // le mieux que l'on ait trouvé, pas forcément intéressant

            best_move  = moves[index].show(output);  // ok, voilà un bon coup à conserver
            best_code  = moves[index].code();

            // on a trouvé un meilleur coup pour nous
            if (score > alpha)
            {

                // Ce coup est trop bon pour l'adversaire
                // comme on appelle la routine avec beta=+MAX_SCORE
                // on n'a jamais score >= beta
//                if(score >= beta)
//                {
//                    // non, ce coup est trop bon pour l'adversaire

//                    // Killer Heuristic
//                    // On stocke 2 Killers (ce qui semble suffisant).
//                    // Lors de la prochaine génération de coups, si on trouve un coup
//                    // qui est un Killer, on va modifier son score
//                    //  ->add_quiet_move
//                    // Puis quand dans la recherche alpha-beta, lorsqu'on cherche
//                    // le meilleur coup (PickNextMove), les 2 Killers Move
//                    // vont remonter vers la tête.

//                    if (moves[index].capture() == false)
//                    {
//                        searchKillers[1][positions->ply] = searchKillers[0][positions->ply];
//                        searchKillers[0][positions->ply] = moves[index].code();
//                    }

////                    hashtable.store(positions->hash, best_code, beta, HASH_BETA, depth, positions->ply);
//                    return beta;
//                }

                alpha      = score;
                do_PVS     = true;

                // alpha cutoff
                // https://www.chessprogramming.org/History_Heuristic
                if (moves[index].capture() == false)
                {
                    searchHistory[positions->side_to_move][moves[index].type()][moves[index].dest()] += (1 << depth); //  pow(2.0, depth) = 2^depth
                }

                /* update the PV */
                pv[positions->ply][positions->ply] = moves[index].show(output);
                if (logTactics)
                    pv[positions->ply][positions->ply] += echec;
                for (int j = positions->ply + 1; j < pv_length[positions->ply + 1]; ++j)
                    pv[positions->ply][j] = pv[positions->ply + 1][j];
                pv_length[positions->ply] = pv_length[positions->ply + 1];

             } // score > alpha

       } // meilleur score

   //     printf("move %d : %s ; score=%d temp_score=%d best_move=%s best_code=%d\n", index, moves[index].show().c_str(), score, temp_score, best_move.c_str(), best_code);

    }  // boucle sur les coups

    /*
     *
---TS----------------------------A----------------------------------------------------

---------------------S----------------------------------------------------------
                     TS
                     BM

---------------------------------------------S-----------------------------------
                                             TS
                                             BM
                                             A

     *
     *
     * Score
     * Temp Score
     * Best Move = Best Code
     * Alpha
     */


    // On écrit le meilleur coup dans la hashtable
    if (!stopped)
    {
        if (alpha != old_alpha)
            hashtable.store(positions->hash, best_code, temp_score,  HASH_EXACT, depth, positions->ply);
        else
            hashtable.store(positions->hash, best_code, alpha,  HASH_ALPHA, depth, positions->ply);
    }

    return alpha;
}


// https://fr.wikipedia.org/wiki/%C3%89lagage_alpha-b%C3%AAta
// https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning

//=====================================================
//! \brief  Recherche du meilleur coup
//!
//!     Méthode Apha-Beta
//!
//! \param[in]  alpha   borne inférieure
//! \param[in]  beta    borne supérieure
//! \param[in]  depth   profondeur de recherche courante
//! \param[in]  doNull  on va effectuer un Null Move
//! \return Valeur du score
//-----------------------------------------------------
I32 Search::alpha_beta(int alpha, int beta, int depth, bool do_NULL)
{
    //   std::cout << "ab depth =" << depth << std::endl;

    //    assert(CheckBoard(pos));
    assert(beta > alpha);
    assert(depth >= 0);

    std::string echec;
    std::string plus[2] = {"", "+"};
    bool do_PVS = false;
//  bool pv_node = alpha != beta - 1;
    int  old_alpha = alpha;


    // NOTE : si pas de quies, mettre pv_length ici
    //    pv_length[ply] = ply;


    if (depth <= 0) {
        return( quiescence(alpha, beta) );

//TODO store hash ici ? voir Moreland   :  RecordHash(depth, val, hashfEXACT);

        //        nodes++;
        //        return (evaluate(side_to_move));
    }

    nodes++;

    //  Time-out
    if (stopped || checkLimits())
    {
        stopped = true;
        return 0;
    }

    pv_length[positions->ply] = positions->ply;

    /* if this isn't the root of the search tree (where we have
       to pick a move and can't simply return 0) then check to
       see if the position is a repeat. if so, we can assume that
       this line is a draw and return 0. */
    if(positions->is_draw(history))
        return 0;

    /* are we too deep? */
    if(positions->ply >= MAX_PLY - 1)
        return evaluate(positions->side_to_move);
    if (positions->hply >= MAX_HIST - 1)
        return evaluate(positions->side_to_move);

    /* are we in check? if so, we want to search deeper */
    bool in_check = is_in_check(positions->side_to_move);
    if (in_check == true)
        ++depth;

    int score   = -MAX_SCORE;
    U32 ht_move = 0;

    // Recherche de la position actuelle dans la hashtable
    // le test "positions->ply > 0" fait gagner de la perfo

    // on est déjà à ply 1
//    if(positions->ply > 0)
//    {
        bool hfc = hashtable.probe(positions->hash, ht_move, score, alpha, beta, depth, positions->ply);

        if (hfc == true)
        {
            //          positions->hashtable->cut++;

            // Comme on utilise une fonction spéciale pour la racine
            // le score retourné est automatiquement affecté au coup correspondant.
            // La PV line est donc bien correctement mise à jour.

            return score;
        }
        else
        {
            // SI la position a été trouvée dans la hashtable,
            // MAIS que la profondeur est inférieure à depth,
            // ALORS hfc = false, ET ht_move != 0

        }
//    }

    // Null Move Pruning
    //TODO ajouter le test sur les pièces : piece_count >= 5 (toutes les pieces sauf pions
    //TODO ajouter test sur pv_node ??
        // /* !pv_node && */ /* positions->ply && */ (nbr_pieces(positions->side_to_move) > 0) &&

    if( do_NULL && !in_check && depth > NULL_MOVE_R + 1)
    {
        make_null_move();
        score = -alpha_beta(-beta, -beta + 1, depth - 1 - NULL_MOVE_R, false);
        take_null_move();

        if (stopped || checkLimits())
        {
            stopped = true;
            return 0;
        }

        if (score >= beta && abs(score) < IS_MATE)
        {
            return beta;
        }
    }

    // Génération des coups
    gen_moves();

    // Tri des coups de la PV line
    if(ht_move != 0)
    {
        pv_move(ht_move, positions->ply);
    }

    int  legal = 0;                 // indique si on a trouvé des coups
    U32  best_code  = 0;            // meilleur coup à renvoyer
    int  temp_score = -MAX_SCORE;   // meilleur coup local
    score = -MAX_SCORE;

    // Boucle sur tous les coups
    for (int index = first_move[positions->ply]; index < first_move[positions->ply + 1]; ++index)
    {
        // prend le meilleur coup, basé sur le score defini dans la génération des coups
        PickNextMove(index);

        if (make_move(&moves[index]) == false)
            continue;

        if (logTactics)
            echec = plus[is_in_check(positions->side_to_move)];

        legal++;

        if (do_PVS)
        {
            // On peut essayer la PVS (Principal Variation Search)
            score = -alpha_beta(-alpha - 1, -alpha, depth - 1, true);

            // Est-ce que ça a marché ?
//            if (score > alpha)
            if (score > alpha && score < beta)  //TODO test à vérifier
            {
                // Raté, il faut refaire une recherche complète
                score = -alpha_beta(-beta, -alpha, depth - 1, true);
            }
        }
        else
        {
            score = -alpha_beta(-beta, -alpha, depth - 1, true);
        }

        unmake_move(&moves[index]);

        // on retourne 0, car cette valeur ne sera pas prise en compte
        // Return if timeout
        if (stopped || checkLimits())
        {
            stopped = true;
            break;
        }

        // On a trouvé un bon coup
        if(score > temp_score)
        {
            temp_score = score;     // le mieux que l'on ait trouvé, pas forcément intéressant
            best_code  = moves[index].code();   // ok, voilà un bon coup à conserver

            if(score > alpha)
            {
                if(score >= beta)
                {
                                                    // non, ce coup est trop bon pour l'adversaire
                    // Killer Heuristic
                    // On stocke 2 Killers (ce qui semble suffisant).
                    // Lors de la prochaine génération de coups, si on trouve un coup
                    // qui est un Killer, on va modifier son score
                    //  ->add_quiet_move
                    // Puis quand dans la recherche alpha-beta, lorsqu'on cherche
                    // le meilleur coup (PickNextMove), les 2 Killers Move
                    // vont remonter vers la tête.

                    if (moves[index].capture() == false)
                    {
                        searchKillers[1][positions->ply] = searchKillers[0][positions->ply];
                        searchKillers[0][positions->ply] = moves[index].code();
                    }

                    hashtable.store(positions->hash, best_code, beta, HASH_BETA, depth, positions->ply);
                    return beta;
                }

                alpha      = score;
                do_PVS     = true;

                // alpha cutoff
                // https://www.chessprogramming.org/History_Heuristic
                if (moves[index].capture() == false)
                {
                    searchHistory[positions->side_to_move][moves[index].type()][moves[index].dest()] += (1 << depth); //  pow(2.0, depth) = 2^depth
                }

                /* update the PV */
                pv[positions->ply][positions->ply] = moves[index].show(output);
                if (logTactics)
                    pv[positions->ply][positions->ply] += echec;
                for (int j = positions->ply + 1; j < pv_length[positions->ply + 1]; ++j)
                    pv[positions->ply][j] = pv[positions->ply + 1][j];
                pv_length[positions->ply] = pv_length[positions->ply + 1];

            } // score > alpha
        }
    } // boucle sur les coups

    // est-on mat ou pat ?
    if (legal == 0)
    {
        if (in_check)
        {
            // On est en échec, et on n'a aucun coup : on est MAT
            //     std::cout <<  "on est echec" << std::endl;

            return -MAX_SCORE + positions->ply;
        }
        else
        {
            // On n'est pas en échec, et on n'a aucun coup : on est PAT
            return 0;
        }
    }

    // faut-il écrire le coup trouvé dans la hashtable, même si alpha n'est pas amélioré ?
    // d'après les tests : OUI
    // On écrit le meilleur coup dans la hashtable
    if (!stopped)
    {
        if (alpha != old_alpha)
            hashtable.store(positions->hash, best_code, temp_score,  HASH_EXACT, depth, positions->ply);
        else
            hashtable.store(positions->hash, best_code, alpha,  HASH_ALPHA, depth, positions->ply);
    }

    return(alpha);
}

//=============================================================
//! \brief  Recherche jusqu'à obtenir une position calme,
//!         donc sans prise ou promotion.
//!
//!
//-------------------------------------------------------------
I32 Search::quiescence(int alpha, int beta)
{

    assert(beta > alpha);
    //    ASSERT(CheckBoard(pos));

    std::string echec;
    std::string plus[2] = {"", "+"};

    nodes++;

    // Check search limits
    if (stopped || checkLimits())
    {
        stopped = true;
        return 0;
    }

    pv_length[positions->ply] = positions->ply;

    if(positions->is_draw(history))
        return 0;

    /* are we too deep? */
    if(positions->ply >= MAX_PLY - 1)
        return evaluate(positions->side_to_move);
    if (positions->hply >= MAX_HIST - 1)
        return evaluate(positions->side_to_move);

    /* check with the evaluation function */
    I32 score = evaluate(positions->side_to_move);

  //  printf( "quiescence a=%d b=%d score=%d \n", alpha, beta, score);

    assert(score > -MAX_SCORE && score < MAX_SCORE);

    // le score est trop mauvais pour moi, on n'a pas besoin
    // de chercher plus loin
    if(score >= beta)
        return beta;

    // l'évaluation est meilleure que alpha. Donc on peut améliorer
    // notre position. On continue à chercher.
    if(score > alpha)
        alpha = score;

    gen_caps();

    int Legal = 0;
    score = -MAX_SCORE;

    // Boucle sur tous les coups
    for (int index = first_move[positions->ply]; index < first_move[positions->ply + 1]; ++index)
    {
        PickNextMove(index);

        if (make_move(&moves[index]) == false)
            continue;

        if (logTactics)
            echec = plus[is_in_check(positions->side_to_move)];

        Legal++;
        seldepth = positions->ply;
        score = -quiescence(-beta, -alpha);
        unmake_move(&moves[index]);

        if(score > alpha)
        {
            if(score >= beta)
                return beta;

            alpha = score;

            /* update the PV */
            //TODO : faut-il le faire ici ??
            pv[positions->ply][positions->ply] = moves[index].show(output);
            if (logTactics)
                pv[positions->ply][positions->ply] += echec;
            for (int j = positions->ply + 1; j < pv_length[positions->ply + 1]; ++j)
                pv[positions->ply][j] = pv[positions->ply + 1][j];
            pv_length[positions->ply] = pv_length[positions->ply + 1];
        }
    }

    // On ne stocke pas BestMove dans la hashtable
    // car depth=0 (voir fonction 'store')

    // à moins de ne pas utiliser la profondeur

    return alpha;
}

void Search::PickNextMove(int moveNum)
{
    int Score     = -1;
    int bestNum   = moveNum;

    for (int index = moveNum; index < first_move[positions->ply + 1]; ++index)
    {
        if (moves[index].score() > Score)
        {
            Score   = moves[index].score();
            bestNum = index;
        }
    }

    if (moveNum != bestNum)
    {
        std::swap(moves[moveNum], moves[bestNum]);
    }

    //  verif("PickNextMove", positions);
}

//=========================================================
//! \brief  Affichage UCI du résultat de la recherche
//!
//! \param[in]  debut   début de l'intervalle de temps
//! \param[in]  depth   profondeur de la recherche
//---------------------------------------------------------
void Search::show_uci_result(int depth, int seldepth, int bestScore, int elapsed) const
{
    elapsed++;  // évite une division par 0

    // commande envoyée à UCI

    //plante        std::cout.imbue(std::locale("fr"));


    std::cout.imbue( std::locale( std::locale::classic(), new MyNumPunct ) );

    // le point séparateur n'est pas bien reconnu par Arena

    std::cout << "info ";

    if (bestScore > IS_MATE)
    {
        std::cout << "mate " << (MAX_SCORE - bestScore)/2 + 1;  // mate <y>
                                                        // mate in y moves, not plies.
                                                        // If the engine is getting mated use negative values for y.
    }
    else if (bestScore < -IS_MATE)
    {
        std::cout << "mate -" << (bestScore - MAX_SCORE)/2 + 1;
    }
    else
    {
        std::cout << "score cp " << std::right << bestScore;    // the score from the engine's point of view in centipawns
    }

    // r3r2k/2R3pp/pp1q1p2/8/3P3R/7P/PP3PP1/3Q2K1 w - - bm Rxh7+; id "WAC.035";

    std::cout << " depth "    << std::setw(2)  << depth
              << " seldepth " << std::setw(2)  << seldepth
              << " nodes "    << std::setw(14) << nodes
              << " nps "      << std::setw(5)  << std::fixed << std::setprecision(2)  << nodes/elapsed/1000.0
              << " time "     << std::setw(6)  << elapsed/1000.0;

    std::cout <<" pv";
    for (int j = 0; j < pv_length[0]; ++j)
        std::cout << " " << pv[0][j];
    std::cout << std::endl;
}

//=========================================================
//! \brief  Affichage UCI du meilleur coup trouvé
//!
//! \param[in]  name   coup en notation ?
//---------------------------------------------------------
void Search::show_uci_best(const std::string& name) const
{
    // ATTENTION AU FORMAT D'AFFICHAGE
    std::cout << "bestmove " << name << std::endl;
}

/**
 * @brief Returns True if this search has exceeded its given limits
 *
 * Note that to avoid a needless amount of computation, limits are only
 * checked every 4096 calls to _checkLimits() (using the Search::_limitCheckCount property).
 * If Search::_limitCheckCount is not 0, false will be returned.
 *
 * @return True if this search has exceed its limits, true otherwise
 */
//=========================================================
//! \brief  Controle du time-out
//---------------------------------------------------------
bool Search::checkLimits()
{
    if (--limitCheckCount > 0)
    {
        return false;
    }

    limitCheckCount = 4096; // 2048; //TODO 4096 ??
    return timer.checkLimits(nodes);
}

//=========================================================
//! \brief  Initialisation de la recherche de profondeur
//---------------------------------------------------------
void Search::setDepth(int depth)
{
    timer.setSearchInfinite(false);
    timer.setSearchLimitDepth(depth);
    timer.init(positions);
}

//=========================================================
//! \brief  Initialisation du temps de recherche
//---------------------------------------------------------
void Search::setTime(int time)
{
    timer.setSearchInfinite(false);
    timer.setSearchLimitTime(time);
    timer.init(positions);
}

//=========================================================
//! \brief  Initialisation de la recherche infinie
//---------------------------------------------------------
void Search::setInfinite(bool infini)
{
    timer.setSearchInfinite(infini);
    timer.init(positions);
}
