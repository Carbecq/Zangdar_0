
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iomanip>

#include "Search.h"
#include "defines.h"

//====================================================
//! \brief Réalisation d'une série de tests tactiques
//!
//! \param[in]  dmax    profondeur max
//! \param[in]  tmax    temps max en millisecondes
//----------------------------------------------------
void test_tactics(int dmax, int tmax)
{
    // le fichier tests/0000.txt contient la liste des fichiers de test
    // les noms non commentés seront utilisés.

    std::string     str_0000 = "0000.txt";
    std::string     str_home = HOME;
    std::string     str_path = str_home + "tests/" + str_0000;
    std::string     str_file;

    std::string     line;
    std::string     aux;
    std::ifstream   ifs;
    std::string     str;
//    std::vector<std::string> liste1;
//    std::vector<std::string>  poslist;                // liste des positions
//    std::string     aa;
//    char            tag = ';';
    int             numero      = 0;
    int             total_ok    = 0;
    int             total_ko    = 0;
    U64             total_nodes = 0;
    U64             total_time  = 0;

    std::ifstream   f(str_path);
    if (!f.is_open())
    {
        std::cout << "[test_tactics] impossible d'ouvrir le fichier " << str_path << std::endl;
        return;
    }

    Search search;
    search.new_game();

    //-------------------------------------------------

    while (std::getline(f, str))
    {
        // ligne vide
        if (str.empty())
            continue;

        // Commentaire ou espace au début de la ligne
        aux = str.substr(0,1);
        if (aux == "#" || aux == " " || aux== "\n" )
            continue;

        std::cout << "test du fichier : [" << str << "]" << std::endl;

        str_file = str_home + "tests/" + str;
        ifs.open(str_file, std::ifstream::in);
        if (!ifs.is_open())
        {
            std::cout << "[test_tactics] impossible d'ouvrir le fichier [" << str_file << "]" << std::endl;
            continue;
        }

        //---------------------------------------------------
        numero      = 0;
        total_ok    = 0;
        total_ko    = 0;
        total_nodes = 0;
        total_time  = 0;

        // Boucle sur l'ensemble des positions de test
        while (std::getline(ifs, line))
        {
            // ligne vide
            if (line.empty())
                continue;

            // Commentaire ou espace au début de la ligne
            aux = line.substr(0,1);
            if (aux == "#" || aux == "/" || aux == " ")
                continue;

            numero++;
            printf("test %d : ", numero);

            // Extraction des éléments de la ligne
            //  0= position
            //  1= D1 20
            //  2= D2 400
//            liste1 = split(line, tag);

            // Extraction de la position
//            aa = liste1[0];

            // Vérification d'unicité de la position
//            if (std::find(poslist.begin(), poslist.end(), aa) ==  poslist.end())
//                poslist.push_back(aa);
//            else
//            {
//                std::cout << "--------------------position en double : "      << aa << std::endl;
//            }

            // Exécution du test
            if (search.test_tactics(line, dmax, tmax, total_nodes, total_time) == true)
                total_ok++;
            else
                total_ko++;

        } // boucle position

        ifs.close();

        std::cout << "===============================================" << std::endl;
        std::cout << " Fichier " << str << std::endl;
        std::cout << "===============================================" << std::endl;
        std::cout << "total_ok    = " << total_ok << std::endl;
        std::cout << "total_ko    = " << total_ko << std::endl;
        std::cout << "total_nodes = " << total_nodes << std::endl;
        std::cout << "Time        = " << total_time/1000.0 << std::endl;
        std::cout << "nps         = " << std::fixed << std::setprecision(3) << total_nodes/total_time/1000.0 << std::endl;
        std::cout << "===============================================" << std::endl;

    } // boucle fichiers epd

    f.close();
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

    Search search; // = new Search();
    search.new_game();

    // Boucle sur l'ensemble des positions de test
    while (std::getline(file, line))
    {
        // ligne vide
        if (line.empty())
            continue;

        numero++;

        if (search.test_mirror(line) == false)
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
 //   delete search;
}
