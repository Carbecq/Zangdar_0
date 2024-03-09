#include "Uci.h"
#include "TranspositionTable.h"
#include "ThreadPool.h"
#include "PolyBook.h"
#include "Attacks.h"

// Globals
TranspositionTable  transpositionTable(HASH_SIZE);
PolyBook            ownBook;
ThreadPool          threadPool(1, false, true);

#if defined USE_TUNER
#include "Tuner.h"
Tuner               ownTuner;
#endif

extern void init_bitmasks();

int main(int argCount, char* argValue[])
{
    init_bitmasks();
    Attacks::init_masks();

#if defined USE_TUNER
    ownTuner.runTexelTuning();
#else

    Uci* uci = new Uci();
    uci->run();
#endif

    return 0;
}
