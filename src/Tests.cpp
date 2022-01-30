#include "defines.h"
#include "ChessBoard.h"

U64 ChessBoard::test_perft(int depth_max)
{
    //   display_ascii();
    return(search.perft(depth_max));
}
U64 ChessBoard::test_divide(int depth_max)
{
    //   display_ascii();
    return(search.divide(depth_max));
}

void ChessBoard::test_gen(const std::string& line)
{
    search.test_gen(line);
}

void ChessBoard::test_search(int depth, const std::string& line)
{
    search.test_search(depth, line);
}

bool ChessBoard::test_tactics(int depth, const std::string& line)
{
    return(search.test_tactics(depth, line));
}

bool ChessBoard::test_mirror(const std::string& line)
{
    return(search.test_mirror(line));
}
