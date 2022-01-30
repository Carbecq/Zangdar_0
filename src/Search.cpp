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
    _bestMove           = "";
    _bestScore          = 0;

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
    _bestMove       = "";
    _bestScore      = 0;

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

    // iterative deepening
    for(int currentDepth = 1; currentDepth <= targetDepth; ++currentDepth )
    {
        int alpha = -MAX_SCORE;
        int beta  = MAX_SCORE;
        seldepth  = 0;

        int score = search_root(alpha, beta, currentDepth);

        if (stopped)
            break;

        // L'itération s'est terminée sans problème
        // On peut mettre à jour les infos UCI

        int elapsed = 0;
        bool shouldStop = timer.finishOnThisDepth(elapsed, nodes, 0);

        if (logUci)
            show_uci_result(currentDepth, seldepth,  _bestScore, elapsed);

        if (_bestScore > 9000 || _bestScore < -9000)
            break;

        // est-ce qu'on a le temps pour une nouvelle itération ?
        if (shouldStop == true)
            break;
    }

    if (_bestScore > 9000)
        std::cout << "mat en " << MAX_SCORE - _bestScore << " demi-coups" << std::endl;

    if (logUci)
        show_uci_best(_bestMove);

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
int Search::search_root(int alpha, int beta, int depth)
{
#ifdef DEBUG_CLASS
    std::cout << "Search::root" << std::endl;
#endif
    assert(beta > alpha);
    assert(depth >= 0);

    std::string echec;
    std::string plus[2] = {"", "+"};
    bool do_PVS = false;
    HASH_CODE hash_code = HASH_ALPHA;

    nodes++;

    // Time-out ou arrêt
    if (stopped || checkLimits())
    {
        stopped = true;
        return 0;
    }

    pv_length[positions->ply] = positions->ply;

    // Nullité par répétition ou règle des 50 coups
    if((IsRepetition() || positions->fifty >= 100) && positions->ply)
        return 0;

    // Si on est en échec, on cherche plus profondément
    bool check = in_check(positions->side_to_move);
    if (check == true)
        ++depth;

    // génération des coups
    gen_moves();

    int score = -MAX_SCORE;

    // Dans la recherche à la racine, on ne cherche pas
    // dans la hashtable.

    // on est à la racine , donc ply = 0

    std::string bestMove;
    int  legal = 0;         // indique si on a trouvé des coups
    U32  localBestMoveCode = 0;
    int  localBestScore = -MAX_SCORE;

    //TODO comment on trie les coups à la racine ?
    //  MVV VLA : lecture fen
    //  Killer  : gen_moves : ne peut pas marcher à la racine
    //  capture : gen_moves

    for (int index = first_move[positions->ply]; index < first_move[positions->ply + 1]; ++index)
    {
        // prend le meilleur coup, basé sur le score defini dans la génération des coups
        PickNextMove(index);

        if (make_move(&moves[index]) == false)
            continue;

        //std::cout << " >> " << moves[index].show() <<  std::endl;

        if (logTactics)
            echec = plus[in_check(positions->side_to_move)];

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

        if (stopped || checkLimits())
        {
            stopped = true;
            break;
        }

        // On a trouvé un bon coup
        if(score > localBestScore)
        {
            localBestScore    = score;
            localBestMoveCode = moves[index].code();

            // on a trouvé un meilleur coup pour nous
            if (score > alpha)
            {
                bestMove   = moves[index].show(output);

                // Ce coup est trop bon pour l'adversaire
                if(score >= beta) //TODO à faire à la racine ?
                {
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

                    hashtable.store(positions->hash, localBestMoveCode, beta, HASH_BETA, depth, positions->ply);
                    return beta;
                }

                alpha      = score;
                hash_code  = HASH_EXACT;
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

                // Break if we've found a checkmate
//TODO utile ?                if (score == MAX_SCORE)
//                {
//                    break;
//                }
            } // score > alpha
       } // meilleur score
    }  // boucle sur les coups


    //TODO à vérifier
    if (legal == 0)
    {
        _bestMove  = ""; //Move();
        _bestScore = -MAX_SCORE;
        return 0;
    }

    // If the best move was not set in the main search loop
    // alpha was not raised at any point, just pick the first move
    // avaliable (arbitrary) to avoid putting a null move in the
    // transposition table

    //      if (bestMove.getFlags() & Move::NULL_MOVE) {
    if (bestMove.empty() /* .none() */ )
    {
        bestMove = moves[0].show(output);
    }

    // faut-il stocker le coup dans la recherche à la racine ??
    if (!stopped)
    {
        hashtable.store(positions->hash, localBestMoveCode, alpha,  hash_code, depth, positions->ply);

        _bestMove  = bestMove;
        _bestScore = alpha;
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
I32 Search::alpha_beta(int alpha, int beta, int depth, bool doNull)
{
    //   std::cout << "ab depth =" << depth << std::endl;

    //    assert(CheckBoard(pos));
    assert(beta > alpha);
    assert(depth >= 0);

    std::string echec;
    std::string plus[2] = {"", "+"};
    bool do_PVS = false;
    HASH_CODE hash_code = HASH_ALPHA;

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
    if (stopped || checkLimits()) {
        stopped = true;
        return 0;
    }

    pv_length[positions->ply] = positions->ply;

    /* if this isn't the root of the search tree (where we have
       to pick a move and can't simply return 0) then check to
       see if the position is a repeat. if so, we can assume that
       this line is a draw and return 0. */
    if((IsRepetition() || positions->fifty >= 100) && positions->ply)
        return 0;

    /* are we too deep? */
    if(positions->ply >= MAX_PLY - 1)
        return evaluate(positions->side_to_move);
    if (positions->hply >= MAX_HIST - 1)
        return evaluate(positions->side_to_move);

    /* are we in check? if so, we want to search deeper */
    bool check = in_check(positions->side_to_move);
    if (check == true)
        ++depth;

    int score  = -MAX_SCORE;
    U32 PvMove = 0;

    // Recherche de la position actuelle dans la hashtable
    // le test "positions->ply > 0" fait gagner de la perfo

    // on est déjà à ply 1
//    if(positions->ply > 0)
//    {
        bool hfc = hashtable.probe(positions->hash, PvMove, score, alpha, beta, depth, positions->ply);

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
            // ALORS hfc = false, ET PvMove != 0

        }
//    }

    // Mora  :                                                R = 1
    // Drofa : depth >= 3 ; depth ... expression complexe
    // BBC   : depth >= 3 ; alphabeta(depth - 1 - 2         > R = 2
    // Bruce Moreland : R=2 ; alphabeta(depth -1 -R, ...)
    // Vice  : depth >= 4   ; alphabepa(depth - 4           > R = 3

    // Mediocre : R=1 is usually too small, making the null-move search too slow.
    //              And R=3 is too large, making the null-move too likely to miss tactics.

    if( doNull && !check && positions->ply && /* (bigPce[side] > 0) && */ depth > NULL_MOVE_R + 1)
    {
        // https://www.chessprogramming.org/Depth_Reduction_R

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

    gen_moves();

    int  localBestScore = -MAX_SCORE;
    U32  localBestMoveCode = 0;
    int  legal = 0;         // indique si on a trouvé des coups

    score = -MAX_SCORE;

    if(PvMove != 0)
    {
        pv_move(PvMove, positions->ply);
    }

    // Boucle sur tous les coups
    for (int index = first_move[positions->ply]; index < first_move[positions->ply + 1]; ++index)
    {
        // prend le meilleur coup, basé sur le score defini dans la génération des coups
        PickNextMove(index);

        if (make_move(&moves[index]) == false)
            continue;

        if (logTactics)
            echec = plus[in_check(positions->side_to_move)];

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

        if(score > localBestScore)
        {
            localBestScore    = score;
            localBestMoveCode = moves[index].code();

            if(score > alpha)
            {
                if(score >= beta)
                {
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

                    hashtable.store(positions->hash, localBestMoveCode, beta, HASH_BETA, depth, positions->ply);
                    return beta;
                }
                alpha      = score;
                hash_code  = HASH_EXACT;
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

    /* no legal moves? then we're in checkmate or stalemate */
    if (legal == 0)
    {
        if (check)
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

    // https://chess.stackexchange.com/questions/8835/collecting-principal-variations-during-search-and-using-them-in-ordering
    // https://www.chessprogramming.org/Triangular_PV-Table
    // https://vajoletchess.blogspot.com/2013/11/a-modern-way-to-collect-principal.html

    hashtable.store(positions->hash, localBestMoveCode, alpha,  hash_code, depth, positions->ply);

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
    //   std::cout << "quiescence " << std::endl;

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

    if((IsRepetition() || positions->fifty >= 100) && positions->ply)
        return 0;

    /* are we too deep? */
    if(positions->ply >= MAX_PLY - 1)
        return evaluate(positions->side_to_move);
    if (positions->hply >= MAX_HIST - 1)
        return evaluate(positions->side_to_move);

    /* check with the evaluation function */
    I32 Score = evaluate(positions->side_to_move);

    assert(Score > -MAX_SCORE && Score < MAX_SCORE);

    // le score est trop mauvais pour moi, on n'a pas besoin
    // de chercher plus loin
    if(Score >= beta)
        return beta;

    // l'évaluation est meilleure que alpha. Donc on peut améliorer
    // notre position. On continue à chercher.
    if(Score > alpha)
        alpha = Score;

    gen_caps();

    int Legal = 0;
    Score = -MAX_SCORE;

    // Boucle sur tous les coups
    for (int index = first_move[positions->ply]; index < first_move[positions->ply + 1]; ++index)
    {
        PickNextMove(index);

        if (make_move(&moves[index]) == false)
            continue;

        if (logTactics)
            echec = plus[in_check(positions->side_to_move)];

        Legal++;
        seldepth = positions->ply;
        Score = -quiescence(-beta, -alpha);
        unmake_move(&moves[index]);

        if(Score > alpha)
        {
            if(Score >= beta)
                return beta;

            alpha = Score;

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

/* reps() returns the number of times the current position
   has been repeated. It compares the current value of hash
   to previous values. */

int Search::IsRepetition()
{
    int r = 0;

    for (int i = positions->hply - positions->fifty; i < positions->hply; ++i)
    {
        if (history[i].hash == positions->hash)
            r++;
    }

    return r;
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

    std::cout << "info score cp "
              << std::right << bestScore
              << " depth " << std::setw(2)  << depth
              << " seldepth " << std::setw(2)  << seldepth
              << " nodes " << std::setw(14) << nodes
              << " nps "   << std::setw(5) << std::fixed << std::setprecision(2)  << nodes/elapsed/1000.0
              << " time "  << std::setw(6) << elapsed/1000.0;

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
