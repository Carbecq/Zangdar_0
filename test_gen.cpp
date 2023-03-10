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
        if (line.empty())
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

    Board CB(fen);

    std::cout << CB << std::endl;
    std::cout << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    U64 total;

    if (CB.turn() == WHITE)
        total = CB.divide<WHITE>(depth);
    else
        total = CB.divide<BLACK>(depth);

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



