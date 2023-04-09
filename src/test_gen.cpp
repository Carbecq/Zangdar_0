#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <algorithm>

#include "defines.h"
#include "Board.h"
#include <unistd.h>
#include <stdio.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

//====================================================
//! \brief Réalisation d'une série de tests "perft"
//!
//! On va faire, pour chaque position, plusieurs tests
//!
//! \param[in]  dmax    profondeur max
//----------------------------------------------------
void test_suite(const std::string& abc, int dmax)
{
    char buffer[256];
    if (getcwd(buffer, sizeof(buffer)) != NULL)
        printf("Current working directory : %s\n", buffer);

    // Il y a 2 suites perft
    //  perftsuite_ref  : petite suite permettant de voir s'il y a eu perte de perfo
    //  perftsuite_big  : énorme suite pour contrôler le générateur de coups

    std::string     str_home = HOME;
    std::string     str_file = str_home + "tests/perftsuite_" + abc + ".epd";
    std::ifstream file(str_file);
    if (!file.is_open())
    {
        std::cout << "[com_perft] impossible d'ouvrir le fichier perftsuite.epd " << std::endl;
        return;
    }

    std::string     line;
    std::string     str;
    std::vector<std::string> liste1;
    std::vector<std::string> liste2;
    std::string     fen;

    std::string     aux;
    int             depth;
    U64             expected;
    U64             actual;
    U64             total_expected = 0;
    U64             total_actual   = 0;
    int             total_tests    = 0;
    int             passed_tests   = 0;
    int             failed_tests   = 0;
    int             numero         = 0;
    std::vector<std::string>  poslist;                // liste des positions
    std::string     aa;
    int             indice;
    char            tag = ';';
    char            tag2 = ' ';

    Board *CB = new Board();
    auto start = std::chrono::high_resolution_clock::now();

    // Boucle sur l'ensemble des positions de test
    while (std::getline(file, line))
    {
        numero++;

        // ligne vide
        if (line.size() < 3)
            continue;

        // Commentaire ou espace au début de la ligne
        aux = line.substr(0,1);
        if (aux == "/" || aux == " ")
            continue;

        // Extraction des éléments de la ligne
        //  0= position
        //  1= D1 20
        //  2= D2 400
        liste1 = split(line, tag);

        // Extraction de la position
        // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR
        aa = liste1[0];

        // Vérification d'unicité de la position
        if (std::find(poslist.begin(), poslist.end(), aa) ==  poslist.end())
            poslist.push_back(aa);
        else
        {
            std::cout << "--------------------position en double : ligne " << numero << " : " << aa << std::endl;
        }

        fen    = liste1.at(0);                  // position fen

        // nombre de profondeurs possibles
        int nbr_prof = liste1.size() - 1;

        // boucle sur les profondeurs de test
        for (int i=1; i<=nbr_prof; i++)
        {
            CB->set_fen(fen, false);

            aux     = liste1.at(i);                     // "D1 20"
            liste2  = split(aux, tag2);                  // 0= "D1"; 1= "20"

            if (liste2.size() > 1)
            {
                // la profondeur est indiquée : "D1 20"
                indice  = 1;
                aux     = liste2.at(0);                     // "D1"
                depth   = std::stoi(aux.substr(1, aux.size()));   // profondeur = 1
            }
            else
            {
                // la profondeur n'est pas indiquée : "20"
                indice = 0;
                depth  = i;
            }

            if (depth <= dmax)
            {
                aux = liste2.at(indice);                     // "20"
                expected = std::stoull(aux);

                // Exécution du test perft pour cette position et cette profondeur
                if (CB->turn() == WHITE)
                    actual = CB->perft<WHITE>(depth);
                else
                    actual = CB->perft<BLACK>(depth);

                total_expected += expected;
                total_actual   += actual;
                total_tests++;

                if (expected == actual)
                {
                    passed_tests++;
                }
                else
                {
                    std::cout << "ligne=" << numero << " ; depth=" << depth << "  FAILED : attendus=" << expected << " ; trouves=" << actual << std::endl;
                    std::cout << line << std::endl;
                    Board board(fen);
                    failed_tests++;
                    std::cout << board << std::endl;
                    return;
                }
            }
        } // boucle depth
    } // boucle position

    // Elapsed time in milliseconds
    auto end = std::chrono::high_resolution_clock::now();
    auto sec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()/1000.0;

    file.close();

    printf("# Passed       %10u\n",     passed_tests);
    printf("# Failed       %10u\n",     failed_tests);
    printf("# Total        %10u\n",     total_tests);
    printf("Moves Actual   %10lu\n",    total_actual);
    printf("Moves Expected %10lu\n",    total_expected);
    printf("Time           %9.3f s\n",  sec);
    if (sec > 0)
        printf("Million Moves/s        %.3f\n",     (double)total_actual/(double)sec/1000000.0);

    std::cout << "********************" << std::endl;

    delete CB;
}

//========================================================
//! \brief  lancement d'un test perft sur une position
//! \param  depth   profondeur max de recherche
//---------------------------------------------------------
void test_perft(const std::string& str, int depth)
{
    std::string fen;
    std::array<U64, 10> nbr;

    if (str == "k")
    {
        fen = KIWIPETE;
        std::array<U64, 10> nbrk = {1, 48, 2039, 97862, 4085603, 193690690, 8031647685, 0, 0, 0};
        nbr = nbrk;
    }
    else if (str == "s")
    {
        fen = SILVER2;
        std::array<U64, 10> nbrk = {1, 48, 2039, 97862, 4085603, 193690690, 8031647685, 0, 0, 0};
        nbr = nbrk;
    }
    else if (str == "r")
    {
        fen = START_FEN;
        std::array<U64, 10> nbrk = {1, 20, 400, 8902,  197281,   4865609,   119060324,   3195901860,  84998978956,   2439530234167  };
        nbr = nbrk;
    }

    Board CB = Board(fen);

    std::cout << CB.display() << std::endl;
    std::cout << std::endl;

    //    CB.test_rays();


    auto start      = std::chrono::high_resolution_clock::now();
    auto start_time = std::chrono::steady_clock::now();
    U64 total;

    if (CB.turn() == WHITE)
        total = CB.perft<WHITE>(depth);
    else
        total = CB.perft<BLACK>(depth);

    auto end        = std::chrono::high_resolution_clock::now();
    auto end_time   = std::chrono::steady_clock::now();

    auto delta      = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    auto delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    auto sec        = delta.count()/1000.0;
    auto sec_time   = delta_time.count()/1000.0;

    std::cout << "sec      = " << sec << std::endl;
    std::cout << "sec time = " << sec_time << std::endl;

    printf("Total           : %10lu\n",   total);
    if (sec > 0)
        printf("Million Moves/s : %.1f\n",     (double)total/(double)sec/1000000.0);
    if (total == nbr[depth])
        printf("resultat        : OK \n");
    else
        printf("resultat        : >>>>>>>>>>>>>>>>>>>>>>>>>>>> KO : bon = %lu \n", nbr[depth]);

    printf(    "evaluation      : %d \n", CB.evaluate());
}

//========================================================
//! \brief  lancement d'un test perft sur une position
//! \param  depth   profondeur max de recherche
//---------------------------------------------------------
void test_divide(int depth)
{
    std::string fen = START_FEN;
    U64 nbr[10] = {1, 20, 400, 8902,  197281,   4865609,   119060324,   3195901860,  84998978956,   2439530234167  };

    Board board(fen);

    std::cout << board << std::endl;
    std::cout << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    U64 total;

    if (board.turn() == WHITE)
        total = board.divide<WHITE>(depth);
    else
        total = board.divide<BLACK>(depth);

    auto end = std::chrono::high_resolution_clock::now();
    auto sec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()/1000.0;

    printf("Total           : %10lu\n",   total);
    if (sec > 0)
        printf("Million Moves/s : %.1f\n",     (double)total/(double)sec/1000000.0);
    if (total == nbr[depth])
        printf("resultat        : OK \n");
    else
        printf("resultat        : >>>>>>>>>>>>>>>>>>>>>>>>>>>> KO : bon = %lu \n", nbr[depth]);

}

//#include "movegen.h"

void Board::test_rays()
{
#if 0
    std::string fen;
    fen = "8/8/k1K2Br1/8/4B3/8/6b1/8 w - - 0 1";
    fen = "k1N5/8/K2Br3/8/2B3B1/8/4b3/8 w - - 0 1";
    set_fen(fen);

    std::cout << display() << std::endl;
    std::cout << std::endl;


    const auto us   = turn();             // side_to_move
    const auto them = !us;
    const auto ksq  = x_king[us];



    const auto kfile = bitboards::files[Square::file(ksq)];
    const auto krank = bitboards::ranks[Square::rank(ksq)];
    const auto pinned = this->pinned();                 // toutes les pièces clouées
    const auto pinned_horizontal = pinned & krank;      // toutes les pièces clouées horizontalement
    const auto pinned_vertical = pinned & kfile;
    const auto pinned_by_rook = pinned_horizontal | pinned_vertical;   // toutes les pièces clouées par une tour
    const auto pinned_bishop = pinned ^ pinned_by_rook;                // toutes les pièces clouées par un fou


    Bitboard bbaux;
    auto allowed        = occupancy_c<them>();

    //    PrintBB(pinned);
    //    PrintBB(pinned_horizontal);
    //    PrintBB(pinned_vertical);
    //    PrintBB(pinned_by_rook);
    //    PrintBB(~pinned_by_rook);      // toutes les cases sauf les pièces clouées par une tour
    //    PrintBB(pinned_bishop);

    // Bitboard contenant les cases attaquées par un fou
    // situé à la position du roi
    const Bitboard bishop_rays = movegen::bishop_moves(ksq, occupied());

    // Bitboard contenant les cases attaquées par une tour
    // situé à la position du roi
    const Bitboard rook_rays   = movegen::rook_moves(ksq, occupied());


    Bitboard bishop_pinned = ZERO;
    Bitboard rook_pinned   = ZERO;

    auto allowed_depl = non_occupied();

    bbaux = occupancy(us) & bishop_rays;
    printf("---------------------------bishop_rays \n");
    PrintBB(bishop_rays);
    printf("---------------------------occupancy(us) & bishop_rays \n");
    PrintBB(bbaux);


    while (bbaux) {
        int sq = next_square(bbaux);
        const auto bb = square_to_bit(sq);
        const auto blockers = occupied() ^ bb;
        printf("---------------------------blockers \n");
        PrintBB(blockers);

        const auto new_rays  = movegen::bishop_moves(ksq, blockers);
        const auto discovery = new_rays ^ bishop_rays;
        const auto attackers = discovery & (pieces(them, PieceType::Bishop) | pieces(them, PieceType::Queen));

        if (attackers) {
            bishop_pinned |= bb;

            const auto asq = first_square(attackers);
            auto move_mask = (movegen::squares_between(ksq, asq) ^ bb) & allowed_depl;
            printf("---------------------------move_mask \n");
            PrintBB(move_mask);

        }
    }

#endif
}

#include "Search.h"

//========================================================
//! \brief  Affiche tous les coups possibles ainsi que leur valeur
//! \param  depth   profondeur max de recherche
//---------------------------------------------------------
void test_eval(const std::string& fen)
{

//    std::string promobug = "8/p1R5/6p1/3k2Np/7P/5K2/1bp4r/8 b - - 14 60 ";

//    fen = "7k/3q4/8/1r1R2b1/8/3p4/8/7K w - - 0 1";
//    fen = "r3k3/1K6/8/8/8/8/8/8 w q - 0 1";
//    fen = "k7/8/8/1p6/5q2/1Q2P3/8/7K w - - 0 1";
//    fen = "7k/Rp1b3B/2BR3p/2Nr2Qn/q5qb/prqn1b2/1PP1P3/3R1K2 w - - 0 1"; // test mvvlva
//fen = "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1 ";
//fen = "6k1/5ppp/8/1pP5/PP6/4P3/8/6K1 w - - 3 1 ";
//fen = "6k1/4Pppp/8/1pP5/PP6/4P3/4P3/6K1 w - - 3 1 ";

    Board b;
    Timer t;
    OrderingInfo info;
    Search search(b, t, info, false, 0);
    search.test_value(fen);
}

//====================================================
//! \brief Réalisation d'un test controlant si
//! l'évaluation est symétrique.
//!
//----------------------------------------------------
void test_mirror(void)
{
    std::string     str_home = HOME;
    std::string     str_file = str_home + "tests/mirror.epd";

    std::ifstream file(str_file);
    if (!file.is_open())
    {
        std::cout << "[test_mirror] impossible d'ouvrir le fichier " << str_file << std::endl;
        return;
    }

    std::string     line;
    int numero = 0;
    Board board;

    // Boucle sur l'ensemble des positions de test
    while (std::getline(file, line))
    {
        // ligne vide
        if (line.size() < 3)
            continue;

        numero++;

        if (board.test_mirror(line) == false)
        {
            std::cout << "Mirror Fail : " << line << std::endl;
        }
        else
        {
//            std::cout << "Mirror OK : " << line << std::endl;
        }

        if((numero % 1000) == 0)
            std::cout << "position " << numero << std::endl;
    }

    file.close();
}
