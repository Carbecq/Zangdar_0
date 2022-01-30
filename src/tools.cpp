#include <string>
#include <vector>
#include <sstream>

#include "defines.h"

//======================================================================
//! \brief  Découpe une chaine en un vecteur de sous-chaines
//! \param[in]  s           chaine à découper
//! \param[in]  delimiter   séparateur
//! \return                 vecteur contenant les sous-chaines
//----------------------------------------------------------------------
std::vector<std::string> split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}


std::string ascii(Square s)
{
    char a = s%16 + 'a';
    char b = s/16 + '1';
    std::string r = std::string(1, a) + std::string(1, b);
    return(r);
}
