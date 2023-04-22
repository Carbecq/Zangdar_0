#ifndef MOVELIST_H
#define MOVELIST_H

class MoveList;

#include <array>
#include "defines.h"

class MoveList
{
public:
    MoveList();

public:
    void clear() { count = 0; }
    size_t size() const { return count; }
    void swap(size_t i, size_t j);

    std::array<MOVE, MAX_MOVES>  moves;
    std::array<I32,  MAX_MOVES>  values;
    size_t                       count;


private:
};


#endif // MOVELIST_H
