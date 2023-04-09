#include <iostream>
#include <string>

#include "defines.h"

extern void test_perft(const std::string& abc, int dmax);
extern void test_divide(int dmax);
extern void test_suite(const std::string& abc, int dmax);
extern void test_think(const std::string& abc, int dmax, int tmax);
extern void test_eval(const std::string& abc);
extern void test_tactics(int dmax, int tmax);
extern void test_mirror();

void commands()
{
    printf("start \n"); fflush(stdout);

#if __cplusplus >= 201103L
    printf("c++ 11 \n");
#endif
#if __cplusplus >= 201402L
    printf("c++ 14 \n");
#endif
#if __cplusplus >= 201703L
    printf("c++ 17 \n");
#endif
#if __cplusplus >= 202002L
    printf("c++ 20 \n");
#endif
#if __cplusplus > 202002L
    printf("c++ 2x \n");
#endif

#if defined(_MSC_VER)
    printf("msc \n");
#elif defined(__GNUC__)
    printf("gnu \n");
#endif

    std::string  token;
    std::string  fen;
    int          dmax = 6;
    int          tmax = 0;
    while (1)
    {
        std::cout << "Entrez une commande : [h] ";
        std::cin >> token;

        if (token == "q")
            break;

        else if (token == "h")
        {
            std::cout << "q(uit) "      << std::endl;
            std::cout << "v(ersion) "   << std::endl;
            std::cout << "s <ref/big>                   : test suite_perft "                    << std::endl;
            std::cout << "d                             : test divide "                         << std::endl;
            std::cout << "p <r/k/s>                     : test perft <Ref/Kiwipete/Silver2> "   << std::endl;
            std::cout << "t                             : test tactics "                        << std::endl;
            std::cout << "eval                          : test evaluation"                      << std::endl;
            std::cout << "think <k/s/f/q/b>             : test think <Kiwipete/Silver2/Fine70/Quiescence/Bug>"  << std::endl;
            std::cout << "mirror                        : test mirror"                                          << std::endl;
            std::cout << "fen [str]                     : positionne la chaine fen"                             << std::endl;
            std::cout << "dmax [p]                      : positionne la profondeur de recherche"                << std::endl;
            std::cout << "tmax [ms]                     : positionne le temps de recherche en millisecondes"    << std::endl;
        }

        else if (token == "v")
        {
            std::cout << "Zangdar " << VERSION << std::endl;
        }
        else if (token == "s")
        {
            std::string str;
            std::cin >> str;

            test_suite(str, dmax);
        }

        else if (token == "d")
        {
            test_divide(dmax);
        }

        else if (token == "p")
        {
            std::string str;
            std::cin >> str;

            test_perft(str, dmax);
        }

        else if (token == "t")
        {
            test_tactics(dmax, tmax);
        }

        else if(token == "mirror")
        {
            test_mirror();
        }

        else if(token == "eval")
        {
            test_eval(fen);
        }

        else if(token == "think")
        {
            std::string str;
            std::cin >> str;

            test_think(str, dmax, tmax);
        }

        else if(token == "fen")    // positionne l'échiquier
        {
            std::getline(std::cin, fen);
        }
        else if (token == "dmax")
        {
            std::cin >> dmax;
        }

        else if (token == "tmax")
        {
            std::cin >> tmax;
        }

    }
}

