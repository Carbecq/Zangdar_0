#ifndef LIBCHESS_SIDE_HPP
#define LIBCHESS_SIDE_HPP


/*******************************************************
 ** Les couleurs
 **---------------------------------------------------*/

#include <string>
enum Color : int
{
    WHITE = 0,
    BLACK
};

const std::string side_name[] = {"White", "Black"};

//Inverts the color (WHITE -> BLACK) and (BLACK -> WHITE)
constexpr Color operator~(Color C) {
    return Color(C ^ BLACK);
}



#endif
