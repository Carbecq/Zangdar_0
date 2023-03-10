#include <iostream>
#include "Uci.h"
#include "movegen.h"
#include "m42.h"

extern void commands();
extern void inversion();


int main(int argCount, char* argValue[])
{
    movegen::calculate_squares_between();
    M42::init();

    // ATTENTION : ne rien imprimer ici pour ne pas polluer la boucle UCI

    if (argCount > 1 && strcmp("cli", argValue[1]) == 0)
    {
        commands();
        return 0;
    }
    else
    {
        Uci* uci = new Uci();
        uci->run();
    }

    return 0;
}
