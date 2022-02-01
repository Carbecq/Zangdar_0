
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "defines.h"
#include "ChessBoard.h"

//====================================================
//! \brief Réalisation d'une série de tests tactiques
//!
//! \param[in]  dmax    profondeur max
//----------------------------------------------------
void test_tactics(ChessBoard* CB, int dmax)
{
    std::string s = "WinAtChess.epd";
 //   std::string s = "Bratko-Kopec.epd";

    // note la combinaison 6 donne le roque noir coté dame possible
    // c'est le coté roi qu'il faut mettre

    // Bratko-Kopec : solution incorrecte (comb 3)
// https://talkchess.com/forum3/viewtopic.php?f=6&t=57166

 //   string s = "tests_arasan/lapuce2.epd";

    std::ifstream file("tests/" + s);

    if (!file.is_open())
    {
        std::cout << "[test_tactics] impossible d'ouvrir le fichier " << s << std::endl;
        return;
    }

    std::string     line;
    std::string     str;
    std::string     aux;
    std::vector<std::string> liste1;
    std::vector<std::string> liste2;
    std::string     fen;
    int             numero         = 0;
    std::vector<std::string>  poslist;                // liste des positions
    std::string          aa;
    char            tag = ';';
    int             total_ok = 0;
    int             total_ko = 0;

    CB->setOutput(1);
    auto start = std::chrono::high_resolution_clock::now();

    // Boucle sur l'ensemble des positions de test
    while (std::getline(file, line))
    {
        // ligne vide
        if (line.empty())
            continue;

        // Commentaire ou espace au début de la ligne
        aux = line.substr(0,1);
        if (aux == "/" || aux == " ")
            continue;

        numero++;
        printf("test %d : ", numero);

        // Extraction des éléments de la ligne
        //  0= position
        //  1= D1 20
        //  2= D2 400
        liste1 = split(line, tag);

        // Extraction de la position
        aa = liste1[0];

        // Vérification d'unicité de la position
        if (std::find(poslist.begin(), poslist.end(), aa) ==  poslist.end())
            poslist.push_back(aa);
        else
        {
            std::cout << "--------------------position en double : "      << aa << std::endl;
        }

        // Exécution du test
        if (CB->test_tactics(dmax, line) == true)
            total_ok++;
        else
            total_ko++;

    } // boucle position

    // Elapsed time in milliseconds
   auto end = std::chrono::high_resolution_clock::now();
   auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    printf("total_ok = %d \n", total_ok);
    printf("total_ko = %d \n", total_ko);
    printf("Time     = %9.3f s\n",  ms/1000.0);

    file.close();
}
