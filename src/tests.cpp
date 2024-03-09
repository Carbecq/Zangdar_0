#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>      // std::setw

#include "defines.h"
#include "Board.h"
#include "Move.h"


void sort_moves(MoveList& ml);

//====================================================
//! \brief Réalisation d'une série de tests "perft"
//!
//! On va faire, pour chaque position, plusieurs tests
//!
//! \param[in]  dmax    profondeur max
//----------------------------------------------------
void test_suite(const std::string& abc, int dmax)
{
   // char buffer[256];
    //TODO if (getcwd(buffer, sizeof(buffer)) != NULL)
    //    printf("Current working directory : %s\n", buffer);

    // Il y a 2 suites perft
    //  perftsuite_ref  : petite suite permettant de voir s'il y a eu perte de perfo
    //  perftsuite_big  : énorme suite pour contrôler le générateur de coups

    std::string     str_file = Home + "tests/perftsuite_" + abc + ".epd";
    std::ifstream file(str_file);
    if (!file.is_open())
    {
        std::cout << "[test_suite] impossible d'ouvrir le fichier perftsuite.epd " << std::endl;
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
//    std::string     aa;
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
//        aa = liste1[0];

        // Vérification d'unicité de la position
        // à ne faire qu'une fois

        // if (std::find(poslist.begin(), poslist.end(), aa) ==  poslist.end())
        //     poslist.push_back(aa);
        // else
        // {
        //     std::cout << "--------------------position en double : ligne " << numero << " : " << aa << std::endl;
        // }

        fen    = liste1.at(0);                  // position fen

        CB->set_fen(fen, false);
//        int sc = CB->evaluate();
//        printf("%d \n", sc);
//#if 0
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
//#endif
    } // boucle position

    // Elapsed time in milliseconds
    auto end = std::chrono::high_resolution_clock::now();
    auto sec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()/1000.0;

    file.close();

    std::cout << "# Passed       " << std::setw(10) << passed_tests << std::endl;
    std::cout << "# Failed       " << std::setw(10) << failed_tests << std::endl;
    std::cout << "# Total        " << std::setw(10) << total_tests << std::endl;
    std::cout << "Moves Actual   " << std::setw(10) << total_actual << std::endl;
    std::cout << "Moves Expected " << std::setw(10) << total_expected << std::endl;
    std::cout << "Time           " << std::setw(9)  << sec << std::endl;
    if (sec > 0)
        std::cout << "Million Moves/s        " << std::setw(9) << static_cast<double>(total_actual)/static_cast<double>(sec)/1000000.0 << std::endl;

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
    std::array<U64, 11> nbr;

    if (str == "k")
    {
        fen = KIWIPETE;
        std::array<U64, 11> nbrk = {1, 48, 2039, 97862, 4085603, 193690690, 8031647685, 374190009323, 0, 0};
        nbr = nbrk;
    }
    else if (str == "s")
    {
        fen = SILVER2;
        std::array<U64, 11> nbrk = {1, 45, 1927, 82516, 3511858, 146708852, 6179265298, 253624821177, 0, 0, 0};
        nbr = nbrk;
    }
    else if (str == "r")
    {
        fen = START_FEN;
        std::array<U64, 11> nbrk = {1, 20, 400, 8902,  197281,   4865609,   119060324,   3195901860,  84998978956,   2439530234167,  69352859712417  };
        nbr = nbrk;
    }
    else if (str == "p3")
    {
        fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -";
        std::array<U64, 11> nbrk = {1, 14, 191, 2812, 43238, 674624, 11030083, 178633661, 3009794393, 50086749815, 860322602309   };
        nbr = nbrk;
    }
    else if (str == "p4")
    {
        fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
        std::array<U64, 11> nbrk = {1, 6, 264, 9467, 422333, 15833292, 706045033, 27209691363, 1209257875296, 0, 0    };
        nbr = nbrk;
    }
    else if (str == "p5")
    {
        fen = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
        std::array<U64, 11> nbrk = {1, 44, 1486, 62379, 2103487, 89941194, 3048196529, 131724123591, 0, 0, 0    };
        nbr = nbrk;
    }
    else if (str == "p6")
    {
        fen = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
        std::array<U64, 11> nbrk = {1, 46, 2079, 89890, 3894594, 164075551, 6923051137, 287188994746, 11923589843526, 490154852788714, 0     };
        nbr = nbrk;
    }

    /* https://www.chessprogramming.org/Perft_Results
     * http://www.rocechess.ch/perft.html
     */
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

    std::cout << "Total           : " << std::setw(10) << total << std::endl;
    if (sec > 0)
        std::cout << "Million Moves/s : " << std::fixed << std::setprecision(1) << static_cast<double>(total)/static_cast<double>(sec)/1000000.0 << std::endl;
    if (total == nbr[depth])
        std::cout << "resultat        : OK " << std::endl;
    else
        std::cout << "resultat        : >>>>>>>>>>>>>>>>>>>>>>>>>>>> KO : bon = " << nbr[depth] << std::endl;

    std::cout << "evaluation      : " << CB.evaluate() << std::endl;
}

//========================================================
//! \brief  lancement d'un test perft sur une position
//! \param  depth   profondeur max de recherche
//---------------------------------------------------------
void test_divide(const std::string &fen, int depth)
{
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

    std::cout << "Total           : " << std::setw(10) << total << std::endl;
    if (sec > 0)
        std::cout << "Million Moves/s : " << std::fixed << std::setprecision(1) << static_cast<double>(total)/static_cast<double>(sec)/1000000.0 << std::endl;
    if (total == nbr[depth])
        std::cout << "resultat        : OK " << std::endl;
    else
        std::cout << "resultat        : >>>>>>>>>>>>>>>>>>>>>>>>>>>> KO : total = " << total << " ; bon = " << nbr[depth] << std::endl;

}

//========================================================
//! \brief  Affiche tous les coups possibles ainsi que leur valeur
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

    // 4k3/8/8/8/8/8/8/4K2R w K - 0 1 ;D1 15 ;D2 66 ;D3 1197 ;D4 7059 ;D5 133987 ;D6 764643
//    std::string fen2 = "7k/8/1Pp5/4P3/8/3p4/P7/7K b - - 0 1 ";
    std::string bug =
//        "2rqr3/pb5Q/4p1p1/1P1p2k1/3P4/2N5/PP6/1K2R3 w - - 0 28 ";
 //   "rnbqkbnr/ppppp3/5qqq/8/3P4/QQQ5/4PPPP/RNBQKBNR b KQkq - 0 1";
"2rBrb2/3k1p2/1Q4p1/4P3/3n1P1p/2P4P/P6P/1K1R4 w - - 0 39 ";

    Board b;

    b.test_value(fen);
}
// phase = 24 0 : eval = 159 159

//====================================================
//! \brief Réalisation d'un test contrôlant si
//! l'évaluation est symétrique.
//!
//----------------------------------------------------
void test_mirror(void)
{
    std::string     str_file = Home + "tests/mirror.epd";

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

//===========================================================
//! \brief  Test vérifiant la symétrie de l'évaluation
//-----------------------------------------------------------
bool Board::test_mirror(const std::string& line)
{
    int ev1 = 0; int ev2 = 0;
    bool r = true;
//    std::cout << "********************************************************" << std::endl;
//    std::cout << line << std::endl;

    set_fen(line, true);

//    std::cout << display() << std::endl;

    ev1 = evaluate();
//    std::cout << "side = " << side_to_move << " : ev1 = " << ev1 << std::endl;

    mirror_fen(line, true);

    // Note : pour faire le test, il faut soit désactiver le cache
    //        soit faire "Transtable.clear();" pour chaque évaluation

//    std::cout << display() << std::endl;
    ev2 = evaluate();
//    std::cout << "side = " << side_to_move << " : ev2 = " << ev2 <<  std::endl;

    if(ev1 != ev2)
    {
        std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " << std::endl;
        set_fen(line, true);
        std::cout << display() << std::endl;
        mirror_fen(line, true);
        std::cout << display() << std::endl;
        std::cout << "ev1 = " << ev1 << " ; ev2 = " << ev2 << std::endl;
        r = false;
    }

    return(r);
}

//======================================================
//! \brief  Affiche tous les coups possibles ainsi que leur valeur
//!         L'affichage est fait dans l'ordre défini par la valeur du coup
//------------------------------------------------------
void Board::test_value(const std::string& fen )
{
    set_fen(fen, false);
    std::cout << display() << std::endl;

    MoveList ml;
 //   U32 move;

    printf("side = %s : evaluation = %d \n", side_name[side_to_move].c_str(), evaluate());

    // generate successor moves
//    legal_moves<WHITE>(ml);
//    sort_moves(ml);

    // look over all moves
//    for (int index=0; index<ml.count; index++)
//    {
//        move = ml.moves[index];

//        // execute current move
//        make_move<WHITE>(move);

//        bool doCheck    = is_in_check<BLACK>();

//        printf("side = %s : %s : value=%d score=%d ; ", side_name[side_to_move].c_str(),
//               Move::name(move).c_str(), ml.values[index], evaluate());
//        if (doCheck)
//            printf("blanc fait échec \n");
//        else
//            printf("blanc ne fait pas échec \n");

//        // retract current move
//        undo_move<WHITE>();
//    }
}

#include "MovePicker.h"

//======================================================
//! \brief  Ordonne les captures en fonction de MvvLva
//------------------------------------------------------
void sort_moves(MoveList& ml)
{
    for (size_t i=0; i<ml.count; i++)
    {
        MOVE m = ml.moves[i];
        if (Move::is_capturing(m))
        {
            PieceType piece = Move::piece(m);
            PieceType capt  = Move::captured(m);
            ml.values[i] = MvvLvaScores[capt][piece];
        }
        else
        {
            ml.values[i] = 0;
        }
    }
    for (size_t i=0; i<ml.count-1; i++)
    {
        for (size_t j=i; j<ml.count; j++)
        {
            if (ml.values[j] > ml.values[i] )
            {
                int v = ml.values[i];
                MOVE m = ml.moves[i];

                ml.values[i] = ml.values[j];
                ml.values[j] = v;

                ml.moves[i] = ml.moves[j];
                ml.moves[j] = m;
            }
        }
    }
}

//========================================================
//! \brief  Test de la Static Exchange Evaluation
//--------------------------------------------------------
void test_see()
{
    std::string   str_file = Home + "tests/see.epd";
    std::ifstream file(str_file);
    if (!file.is_open())
    {
        std::cout << "[test_see] impossible d'ouvrir le fichier see.epd " << std::endl;
        return;
    }

    std::string     line;
    std::string     strm;
    MOVE            move;
    std::vector<std::string> liste1;
    std::vector<std::string> liste2;
    std::string     fen;
    int             score;

    std::string     aux;
    int             total_tests    = 0;
    int             passed_tests_b   = 0;
    int             failed_tests_b   = 0;
    int             passed_tests_s   = 0;
    int             failed_tests_s   = 0;
    int             numero         = 0;
    std::vector<std::string>  poslist;                // liste des positions
    std::string     aa;
    char            tag = ';';

    Board board;
    MoveList ml;

    // Boucle sur l'ensemble des positions de test
    while (std::getline(file, line))
    {
        // ligne vide
        if (line.size() < 3)
            continue;

        // Commentaire ou espace au début de la ligne
        aux = line.substr(0,1);
        if (aux == "/" || aux == " " || aux == "#")
            continue;

        numero++;
        printf("%2d : ", numero);

        //    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>%d \n", numero);

        // Extraction des éléments de la ligne
        //  0= position
        //  1= move
        //  2= score
        liste1 = split(line, tag);

        // Extraction de la position
        // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR
        //        aa = liste1[0];

        // Vérification d'unicité de la position
        //        if (std::find(poslist.begin(), poslist.end(), aa) ==  poslist.end())
        //            poslist.push_back(aa);
        //        else
        //        {
        //            std::cout << "--------------------position en double : ligne " << numero << " : " << aa << std::endl;
        //        }

        fen   = liste1[0];                  // position fen
        aux   = liste1[1];

        strm  = aux.substr(1, aux.size());
        score = std::stoi(liste1[2]);

        board.set_fen(fen, false);

        if (board.turn() == WHITE)
            board.legal_moves<WHITE>(ml);
        else
            board.legal_moves<BLACK>(ml);

        move = 0;

        for (int i=0; i<ml.count; i++)
        {
            MOVE m = ml.moves[i];

            std::string str1 = Move::show(m, 1);
            std::string str2 = Move::show(m, 2);
            std::string str3 = Move::show(m, 3);

            //       printf("(%s) : (%s) (%s) (%s) \n", strm.c_str(), str1.c_str(), str2.c_str(), str3.c_str());

            if (strm==str1 || strm==str2 || strm==str3)
            {
                move = m;
                break;
            }
        }
        if (move)
        {
            bool v = board.fast_see(move, 0);
        //    int  s = board.see(move);

            //    int  s = board.see(move);

            //        printf("%2d : %s : (%s) see=%d sees=%d score=%d \n", numero, fen.c_str(), strm.c_str(), v, s, score);

            if (Move::is_capturing(move))
                printf("C");
            if (Move::is_promoting(move))
                printf("P");
            if (Move::is_enpassant(move))
                printf("E");
            if (Move::is_castling(move))
                printf("K");

            if ((v==true && score>=0) || (v==false && score<0))
            {
                printf(" : OK ");
                passed_tests_b++;
            }
            else
            {
                printf(" : %s : (%s) seeB=%d score=%d ", fen.c_str(), strm.c_str(), v, score);
                failed_tests_b++;
            }

//            if ((s>=0 && score>=0) || (s<0 && score<0))
//            {
//                printf(" : OK \n");
//                passed_tests_s++;
//            }
//            else
//            {
//                printf(" : %s : (%s) seeS=%d score=%d \n", fen.c_str(), strm.c_str(), s, score);
//                failed_tests_s++;
//            }
            printf("\n");

            total_tests++;
        }
        else
        {
            printf("coup non trouvé %s \n", strm.c_str());
            printf("%s \n", fen.c_str());
            for (int i=0; i<ml.count; i++)
            {
                MOVE m = ml.moves[i];

                std::string str1 = Move::show(m, 1);
                std::string str2 = Move::show(m, 2);
                std::string str3 = Move::show(m, 3);

                printf("(%s) : (%s) (%s) (%s) \n", strm.c_str(), str1.c_str(), str2.c_str(), str3.c_str());
            }
        }
    } // boucle position

    file.close();

    printf("# Passed B     %10u\n",     passed_tests_b);
    printf("# Passed S     %10u\n",     passed_tests_s);
    printf("# Failed B     %10u\n",     failed_tests_b);
    printf("# Failed S     %10u\n",     failed_tests_s);
    printf("# Total        %10u\n",     total_tests);

    std::cout << "********************" << std::endl;

}

