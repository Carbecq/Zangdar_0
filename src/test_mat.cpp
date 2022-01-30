#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "defines.h"
#include "ChessBoard.h"

//====================================================
//! \brief Réalisation d'une série de tests de mat
//!
//! \param[in]  dmax    profondeur max
//----------------------------------------------------
void test_mat(ChessBoard* CB, int dmax)
{
    std::ifstream file("../Tests/faciles/suite.epd");

    if (!file.is_open())
    {
        std::cout << "[test_mat] impossible d'ouvrir le fichier faciles/suite.epd " << std::endl;
        return;
    }

    std::string     line;
    std::string     str;
    std::vector<std::string> liste1;
    std::vector<std::string> liste2;
    std::string     fen;

    std::string     aux;
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
        //        liste1 = line.split(tag);
        liste1 = split(line, tag);

        // Extraction de la position
        // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR
        aa = liste1[0];

        // Vérification d'unicité de la position
        if (std::find(poslist.begin(), poslist.end(), aa) ==  poslist.end())
            poslist.push_back(aa);
        else
        {
            std::cout << "--------------------position en double : "      << aa << std::endl;
        }

        fen    = liste1[0];                  // position fen

        // Exécution du test
  //      CB->test_mat(dmax, fen);

    } // boucle position


    file.close();
}

void test_gen(ChessBoard* CB)
{

    //The_ChessBoard->test_gen("4k3/8/8/8/8/8/8/4K2R w K - 0 1 ;D1 15 ;D2 66 ;D3 1197 ;D4 7059 ;D5 133987 ;D6 764643");
    CB->test_gen
            ("4k3/8/8/8/3n1p1p/1p1n1P1P/1P1P1PBP/2BR1KBB w - - 0 1");



//    The_ChessBoard->test_gen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1");
//    The_ChessBoard->test_gen(START_FEN);
//    The_ChessBoard->test_gen("8/8/8/8/8/8/R7/R3K2k w Q - 0 1 ;");
//    The_ChessBoard->test_gen("R7/P4k2/8/8/8/8/r7/6K1 w - - 0 1");

}
