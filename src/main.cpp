#include <iostream>
#include "Uci.h"
#include "movegen.h"
#include "m42.h"
#include "TranspositionTable.h"
#include "PolyBook.h"
#include "ThreadPool.h"

extern void inversion();
extern void test_poly();

// Globals
TranspositionTable  Transtable(DEFAULT_HASH_SIZE);
PolyBook            Book;
ThreadPool          threadPool;

int main(int argCount, char* argValue[])
{
    movegen::calculate_squares_between();
    M42::init();

    Uci* uci = new Uci();
    uci->run();

    return 0;
}
