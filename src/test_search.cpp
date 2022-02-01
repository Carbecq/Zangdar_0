#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "defines.h"
#include "ChessBoard.h"


void test_search(ChessBoard* CB, int dmax)
{
    // https://www.talkchess.com/forum3/viewtopic.php?t=32532

    std::string s = "silver.epd";

 //   string s = "tests_arasan/lapuce2.epd";

    std::ifstream file("tests/" + s);

    if (!file.is_open())
    {
        std::cout << "[test_search] impossible d'ouvrir le fichier " << s << std::endl;
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
//        if (std::find(poslist.begin(), poslist.end(), aa) ==  poslist.end())
//            poslist.push_back(aa);
//        else
//        {
//            std::cout << "--------------------position en double : "      << aa << std::endl;
//        }

        // Exécution du test
        CB->test_search(dmax, line);

    } // boucle position

    // Elapsed time in milliseconds
   auto end = std::chrono::high_resolution_clock::now();
   auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

   printf("Time     = %9.3f s\n",  ms/1000.0);

   file.close();
}

void test_mirror(ChessBoard* CB)
{
    std::string s = "mirror.epd";
// WinAtChess
    std::ifstream file("tests/" + s);

    if (!file.is_open())
    {
        std::cout << "[test_mirror] impossible d'ouvrir le fichier " << s << std::endl;
        return;
    }

    std::string     line;
    int numero = 0;

    CB->init();
    CB->setOutput(1);

    // Boucle sur l'ensemble des positions de test
    while (std::getline(file, line))
    {
        // ligne vide
        if (line.empty())
            continue;

        numero++;

        if (CB->test_mirror(line) == false)
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

}

void test_position(ChessBoard* CB, int dmax)
{

    CB->setOutput(1);

    // i implemented MVV/LVA ordering and quiescence search (a search of depth 10 from here "8/1k6/4r1q1/1K1b3p/5N2/8/8/8 w - - 0 1" went from 13s to less than 1!!)

    char aa[100] = "8/1k6/4r1q1/1K1b3p/5N2/8/8/8 w - - 0 1";

    char SILVER2[100] = "r1b1k2r/ppppnppp/2n2q2/2b5/3NP3/2P1B3/PP3PPP/RN1QKB1R w KQkq - 0 1";

    char bug[100] = "QQ6/Qp5k/8/8/8/8/8/7K w - - 0 1";

    char fine70[100] = "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1";

char finale[100] = "8/6R1/2k5/6P1/8/8/4nP2/6K1 w - - 1 41"; // mat en 20 : Kg2
char bug2[100] = "r5r1/pQ5p/1qp2R2/2k1p3/4P3/2PP4/P1P3PP/6K1 w - - 0 1";
char promo[100] = "8/3P2pp/8/b5B1/k7/8/6PP/6K1 w - - 0 1";

    CB->test_search(dmax, SILVER2);


}
