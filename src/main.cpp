#include <iostream>
#include <string>
#include <cassert>
#include "defines.h"
#include "ChessBoard.h"
#include "Uci.h"

extern void commands();
static void init_all();

//FILE* fp = nullptr;

int main(int argc, char *argv[])
{
    // INFO : par défaut WIN32 est défini

    init_all();

//    fp = fopen("log.txt", "w");
//    setvbuf( fp, (char *)NULL, _IONBF, 0 ); // pas de buffering
//    setbuf(fp, NULL);

    commands();

//    fclose(fp);
    return(0);
}

void init_all(void)
{
}

