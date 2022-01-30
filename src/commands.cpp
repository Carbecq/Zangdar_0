#include <iostream>
#include <string>

#include "defines.h"
#include "Uci.h"
#include "ChessBoard.h"

extern void test_perft(ChessBoard *CB, int dmax);
extern void test_divide(ChessBoard *CB, int dmax);
extern void test_mat(ChessBoard *CB, int dmax);
extern void test_gen(ChessBoard *CB);
extern void test_tactics(ChessBoard *CB, int dmax);
extern void test_search(ChessBoard *CB, int dmax);
extern void test_mirror(ChessBoard *CB);
extern void test_position(ChessBoard *CB, int dmax);


static void console();

void commands()
{
    std::string  token;

    while (1)
    {
        std::cout << "Entrez une commande : [help] ";
        std::cin >> token;

        if (token == "uci")
        {
            Uci* uci = new Uci();
            uci->run();
 //           if(info->quit == TRUE)
 //               break;
            continue;
        }

        else if (token == "q")
            break;

        else if (token == "cli")
        {
            console();
            continue;
        }

        else if (token == "help")
        {
            std::cout << "q(uit) / v(ersion) / cli " << std::endl;
        }

        else if (token == "v")
        {
            std::cout << "Zangdar " << VERSION << std::endl;
        }
    }
}

//=========================================================
//  commandes interactives
//---------------------------------------------------------
void console()
{
    std::string  token;

    ChessBoard* CB = new ChessBoard();
    CB->init();


    while (1)
    {
        std::cout << "CLI : [help] ";
        std::cin >> token;

        if (token == "q")
            break;

        else if (token == "help")
        {
            std::cout << "q(uit) / p(erft) n / t(actique) n / s(earch) n /" << std::endl;
        }

        else if (token == "p")
        {
            int dmax;
            std::cin >> dmax;

            test_perft(CB, dmax);
        }

        else if (token == "d")
        {
            int dmax;
            std::cin >> dmax;

            test_divide(CB, dmax);
        }

        else if (token == "mat")
        {
            int dmax;
            std::cin >> dmax;

            test_mat(CB, dmax);
        }

        else if (token == "gen")
        {
            test_gen(CB);
        }

        else if (token == "t")
        {
            int dmax;
            std::cin >> dmax;

            test_tactics(CB, dmax);
        }

        else if (token == "s")
        {
            int dmax;
            std::cin >> dmax;

            test_search(CB, dmax);
        }

        else if (token == "o")
        {
            int dmax;
            std::cin >> dmax;

            test_position(CB, dmax);
        }

        else if(token == "mirror")
        {
            test_mirror(CB);
        }

        else if(token == "display")
            CB->search.display_ascii();

        else if(token == "think")
            CB->search.think();

        else if(token == "depth")
        {
            int dmax;
            std::cin >> dmax;
            CB->search.setDepth(dmax);
        }

        else if(token == "time")    // temps de réflexion en secondes
        {                           // note : la profondeur sera imposée au MAX
            int tmax;
            std::cin >> tmax;
            CB->search.setTime(tmax*1000);
        }

        else if(token == "infini")    // temps et profondeur sans limite
        {
            int infini;
            std::cin >> infini;
            CB->search.setInfinite(infini);
        }

        else if(token == "fen")    // positionne l'échiquier
        {
            std::string fen;
            std::getline(std::cin, fen);

            CB->search.init_fen(fen);
        }

        else if(token == "eval")    // évaluation de la position
        {
            std::cout << "value = " << CB->search.evaluate(WHITE) << std::endl;
        }

        else if(token == "log")    // évaluation de la position
        {
            int u, s, t;
            std::cin >> u;
            std::cin >> s;
            std::cin >> t;

            CB->search.setLogUci(u);
            CB->search.setlogSearch(s);
            CB->search.setlogTactics(t);
        }

        else if(token == "bench")    // positionne l'échiquier
        {
            int dmax;
            std::cin >> dmax;

            CB->search.bench(dmax);
        }
    }


    delete CB;
}
