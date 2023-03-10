#include <thread>
#include "Search.h"
#include <locale>
#include <iomanip>
#include "move.h"


//=============================================
//! \brief  Constructeur
//---------------------------------------------
Search::Search()
{
    board = nullptr;
    timer = nullptr;

    stopped             = false;
    nodes               = 0;
    logUci              = true;
    logSearch           = true;
    logTactics          = false;
    output              = 4;
}

//=============================================
//! \brief  Destructeur
//---------------------------------------------
Search::~Search()
{
}

//=============================================
//! \brief  Remise à zéro
//---------------------------------------------
void Search::reset()
{
    board->reset();
}

//=============================================
//! \brief  RInitialisation depuis une chaine FEN
//---------------------------------------------
void Search::init_fen(const std::string& fen)
{
    board->set_fen(fen, logTactics);
}

//======================================================
//! \brief Initialisation au début d'une recherche
//!        dans le cadre d'une partie
//!     Ne pas confondre avec la fonction new_game,
//!     qui remet à 0 la hashtable.
//------------------------------------------------------
void Search::new_search()
{
    stopped         = false;
    nodes           = 0;

    // game_clock n'est pas modifié, car on est toujours
    // dans la partie

    for (int i=0; i<2; i++) {
        for(int index = 0; index < 6; ++index) {
            for(int index2 = 0; index2 < 64; ++index2) {
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

//=========================================================
//! \brief  Affichage UCI du résultat de la recherche
//!
//! \param[in] depth        profondeur de la recherche
//! \param[in] best_score   meilleur score
//! \param[in] elapsed      temps passé pour la recherche, en millisecondes
//---------------------------------------------------------
void Search::show_uci_result(int depth, int best_score, U64 elapsed, MOVE* pv) const
{
    elapsed++;  // évite une division par 0
    int l = 10;
    // commande envoyée à UCI
    // voir le document : uci_commands.txt

    //plante        std::cout.imbue(std::locale("fr"));

    // en mode UCI, il ne faut pas mettre les points de séparateur,
    // sinon Arena, n'affiche pas correctement le nombre de nodes
//    std::cout.imbue( std::locale( std::locale::classic(), new MyNumPunct ) );
//    l = 12;

    std::cout << "info ";

    /*
     * score
     *       cp <x>
     *           the score from the engine's point of view in centipawns.
     *       mate <y>
     *           mate in y moves, not plies.
     *           If the engine is getting mated use negative values for y.
     */


    if (best_score >= MAX_EVAL)
    {
        std::cout << "mate " << std::setw(2) << (MATE - best_score)/2 + 1;
        std::cout <<  "      ";
    }
    else if (best_score <= -MAX_EVAL)
    {
        std::cout << "mate " << std::setw(2) << (-MATE - best_score) / 2;
        std::cout << "      ";
    }
    else
    {
        std::cout << "score cp " << std::right << std::setw(4) << best_score;    // the score from the engine's point of view in centipawns
    }

    // nodes    : noeuds calculés
    // nps      : nodes per second searched
    // time     : the time searched in ms

    std::cout << " depth "    << std::setw(2) << depth
 //             << " seldepth " << std::setw(2) << seldepth
              << " nodes "    << std::setw(l) << nodes
              << " nps "      << std::setw(7) << nodes*1000/elapsed
              << " time "     << std::setw(6) << elapsed;

    std::cout <<" pv";
    for (MOVE* p=pv; *p!=0; p++)
        std::cout << " " << Move::name(*p);

    std::cout << std::endl;
}

//=========================================================
//! \brief  Affichage UCI du meilleur coup trouvé
//!
//! \param[in]  name   coup en notation UCI
//---------------------------------------------------------
void Search::show_uci_best(const MOVE best_move) const
{
    // ATTENTION AU FORMAT D'AFFICHAGE
    std::cout << "bestmove " << Move::name(best_move) << std::endl;
}

//=========================================================
//! \brief  Mise à jour de la Principal variation
//!
//! \param[in]  name   coup en notation UCI
//---------------------------------------------------------
void Search::update_pv(MOVE *dst, MOVE *src, MOVE move) const
{
    *dst++ = move;
    while ((*dst++ = *src++))
      ;
}

//=========================================================
//! \brief  Controle du time-out
//! \return Retourne "true" si la recherche a dépassé sa limite de temps
//!
//! De façon à éviter un nombre important de calculs , on ne fera
//! ce calcul que tous les 4096 coups.
//---------------------------------------------------------
bool Search::check_limits()
{
    // Every 4096 nodes, check if our time has expired.
    if ((nodes & 4095) == 0)
        return timer->checkLimits();
    return false;
}

//=========================================================
//! \brief  Initialisation de la recherche de profondeur
//---------------------------------------------------------
void Search::setDepth(int depth)
{
    timer->setSearchInfinite(false);
    timer->setSearchLimitDepth(depth);
    timer->setSearchLimitTime(0);
}

//=========================================================
//! \brief  Initialisation du temps de recherche
//---------------------------------------------------------
void Search::setTime(int time)
{
    timer->setSearchInfinite(false);
    timer->setSearchLimitTime(time);
    timer->setSearchLimitDepth(0);
}

//=========================================================
//! \brief  Initialisation de la recherche infinie
//---------------------------------------------------------
void Search::setInfinite(bool infini)
{
    timer->setSearchInfinite(infini);
}

constexpr int MvvLvaScores[6][6] = {
    {16, 15, 14, 13, 12, 11}, // victim Pawn
    {26, 25, 24, 23, 22, 21}, // victim Knight
    {36, 35, 34, 33, 32, 31}, // victim Bishop
    {46, 45, 44, 43, 42, 41}, // vitcim Rook
    {56, 55, 54, 53, 52, 51}, // victim Queen
    { 0,  0,  0,  0,  0,  0}  // victim King
};

//=========================================================
//! \brief  Donne un bonus aux coups, de façon à les trier
//!
//! A typical move ordering consists as follows:
//!
//! PV-move of the principal variation from the previous iteration of an iterative deepening framework for the leftmost path, often implicitly done by 2.
//! Hash move from hash tables
//! Winning captures/promotions
//! Equal captures/promotions
//! Killer moves (non capture), often with mate killers first
//! Non-captures sorted by history heuristic and that like
//! Losing captures (* but see below)
//!
//! https://www.chessprogramming.org/Move_Ordering
//---------------------------------------------------------
void Search::order_moves(int ply, MoveList& move_list, U32 PvMove)
{
    for (int index=0; index<move_list.count; index++)
    {
        move_list.values[index] = 0;
        U32 move = move_list.moves[index];

        if(PvMove && move == PvMove)
        {
            move_list.values[index] = ( 2000000 );
        }
        else if (Move::is_capturing(move))
        {
            // capture
            PieceType piece    = Move::piece(move);
            PieceType captured = Move::captured(move);
            move_list.values[index] = MvvLvaScores[captured][piece] + 1000000;
        }
        else
        {
            // quiet move
            if (searchKillers[0][ply] == move)
                move_list.values[index] = 900000;
            else if (searchKillers[1][ply] == move)
                move_list.values[index] = 800000;
            else
            {
                PieceType piece = Move::piece(move);
                int dest = Move::dest(move);
                move_list.values[index] = searchHistory[board->side_to_move][piece][dest] ;
            }
        }
    }
}

std::string pchar[6] = {"Pion", "Cavalier", "Fou", "Tour", "Dame", "Roi"};
//void Search::verify_MvvLva()
//{
//    for(int Victim = PieceType::Pawn; Victim <= PieceType::King; ++Victim)
//    {
//        for(int Attacker = PieceType::Pawn; Attacker <= PieceType::King; ++Attacker)
//        {
//            printf("%10s prend %10s = %d\n", pchar[Attacker].c_str(), pchar[Victim].c_str(), MvvLvaScores[Victim][Attacker]);
//        }
//    }

//}





