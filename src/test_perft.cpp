#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>

#include "defines.h"
#include "ChessBoard.h"

#include <unistd.h>
#include <stdio.h>
#include <limits.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

//====================================================
//! \brief Réalisation d'une série de tests "perft"
//!
//! On va faire, pour chaque position, plusieurs tests
//!
//! \param[in]  dmax    profondeur max
//----------------------------------------------------
void test_perft(ChessBoard* CB, int dmax)
{
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != NULL)
        printf("Current working directory : %s\n", buffer);


    std::ifstream file("tests/perftsuite.epd");
    if (!file.is_open())
    {
        std::cout << "[com_perft] impossible d'ouvrir le fichier perftsuite.epd " << std::endl;
        return;
    }




    std::string     line;
    std::string     str;
    //clock_t         start, end;
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
    //    vector<string>  poslist;                // liste des positions
    std::string     aa;
    int             indice;
    char            tag = ';';
    char            tag2 = ' ';


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
        //        if (std::find(poslist.begin(), poslist.end(), aa) ==  poslist.end())
        //            poslist.push_back(aa);
        //        else
        //        {
        //            std::cout << "--------------------position en double : "      << aa << std::endl;
        //        }

        fen    = liste1.at(0);                  // position fen

        // nombre de profondeurs possibles
        int nbr_prof = liste1.size() - 1;

        // boucle sur les profondeurs de test
        for (int i=1; i<=nbr_prof; i++)
        {
            CB->init(fen);
    //        The_ChessBoard->display_ascii();

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
                actual = CB->test_perft(depth);

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
                    failed_tests++;
                }
            }
        } // boucle depth
    } // boucle position

    // Elapsed time in milliseconds
   auto end = std::chrono::high_resolution_clock::now();
   auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    file.close();

    printf("# Passed       %10u\n",     passed_tests);
    printf("# Failed       %10u\n",     failed_tests);
    printf("# Total        %10u\n",     total_tests);
    printf("Moves Actual   %10llu\n",   total_actual);
    printf("Moves Expected %10llu\n",   total_expected);
    printf("Time           %9.3f s\n",  ms/1000.0);
    printf("Moves/s        %.3f\n",     (total_actual/ms)/1000.0);

    std::cout << "********************" << std::endl;
}

//====================================================
//! \brief Réalisation d'une série de tests "perft"
//!
//! On va faire, pour chaque position, plusieurs tests
//!
//! \param[in]  dmax    profondeur max
//----------------------------------------------------
void test_divide(ChessBoard* CB, int dmax)
{
    std::ifstream file("../Tests/perft/perftsuite.epd");
    if (!file.is_open())
    {
        std::cout << "[com_perft] impossible d'ouvrir le fichier perftsuite.epd " << std::endl;
        return;
    }
    std::cout << "divide " << std::endl;

    std::string     line;
    std::string     str;
    //clock_t         start, end;
    double          timespan;
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
    //    vector<string>  poslist;                // liste des positions
    std::string     aa;
    int             indice;
    char            tag = ';';
    char            tag2 = ' ';

  //  start = clock();

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
        //        if (std::find(poslist.begin(), poslist.end(), aa) ==  poslist.end())
        //            poslist.push_back(aa);
        //        else
        //        {
        //            std::cout << "--------------------position en double : "      << aa << std::endl;
        //        }

        fen    = liste1.at(0);                  // position fen

        // nombre de profondeurs possibles
        int nbr_prof = liste1.size() - 1;

        // boucle sur les profondeurs de test
        for (int i=1; i<=nbr_prof; i++)
        {
            CB->init(fen);
    //        The_ChessBoard->display_ascii();

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
                actual = CB->test_divide(depth);

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
                    failed_tests++;
                }
            }
        } // boucle depth
    } // boucle position

//    end = clock();
//    timespan = (double)(end - start) / CLOCKS_PER_SEC;

    file.close();

    printf("# Passed       %10u\n",     passed_tests);
    printf("# Failed       %10u\n",     failed_tests);
    printf("# Total        %10u\n",     total_tests);
    printf("Moves Actual   %10llu\n",   total_actual);
    printf("Moves Expected %10llu\n",   total_expected);
    printf("Time           %9.3fs\n",   timespan);
    printf("Moves/s        %.3f\n",     (total_actual/timespan)/1000000.0);

    std::cout << "********************" << std::endl;
}
