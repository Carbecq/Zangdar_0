#include "Uci.h"
#include "Attacks.h"
#include "TranspositionTable.h"
#include "ThreadPool.h"
#include "PolyBook.h"

// Globals
TranspositionTable  transpositionTable(HASH_SIZE);
PolyBook            ownBook;
ThreadPool          threadPool(1, false, true);

int main(int argCount, char* argValue[])
{
    Attacks::init();

    Uci* uci = new Uci();
    uci->run();

    return 0;
}
