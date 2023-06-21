#include <iostream>
#include "Uci.h"
#include "MoveGen.h"
#include "M42.h"
#include "TranspositionTable.h"
#include "PolyBook.h"
#include "ThreadPool.h"

// Globals
TranspositionTable  Transtable(DEFAULT_HASH_SIZE);
bool                UseSyzygy = false;
PolyBook            Book;
ThreadPool          threadPool;

int main(int argCount, char* argValue[])
{
    MoveGen::calculate_squares_between();
    M42::init();

    Uci* uci = new Uci();
    uci->run();

    return 0;
}
