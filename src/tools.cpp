#include <string>
#include <vector>
#include <iostream>
#include <bitset>
#include <sstream>

#include "defines.h"
#include "evaluation.h"

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


//======================================
//! \brief Ecriture en binaire
//--------------------------------------
void binary_print(U32 code)
{
    //    U32 code = moves[index].code();
    std::string binary = std::bitset<32>(code).to_string(); //to binary
    std::cout<<"code   " << binary<< std::endl;
}

//==============================================
//  fonction pour inverser les tables dans evaluation

#include "Square.h"
void inversion()
{
    printf("constexpr int mg_pawn_table[64] = {\n");
    for (int r=7; r>=0; r--)
    {
        for (int f=0; f<8; f++)
            printf("%5d, ", mg_pawn_table[Square::square(f, r)]);
        printf("\n");
    }
    printf("};\n\n");

    printf("constexpr int eg_pawn_table[64] = {\n");
    for (int r=7; r>=0; r--)
    {
        for (int f=0; f<8; f++)
            printf("%5d, ", eg_pawn_table[Square::square(f, r)]);
        printf("\n");
    }
    printf("};\n\n");

    printf("constexpr int mg_knight_table[64] = {\n");
    for (int r=7; r>=0; r--)
    {
        for (int f=0; f<8; f++)
            printf("%5d, ", mg_knight_table[Square::square(f, r)]);
        printf("\n");
    }
    printf("};\n\n");

    printf("constexpr int eg_knight_table[64] = {\n");
    for (int r=7; r>=0; r--)
    {
        for (int f=0; f<8; f++)
            printf("%5d, ", eg_knight_table[Square::square(f, r)]);
        printf("\n");
    }
    printf("};\n\n");

    printf("constexpr int mg_bishop_table[64] = {\n");
    for (int r=7; r>=0; r--)
    {
        for (int f=0; f<8; f++)
            printf("%5d, ", mg_bishop_table[Square::square(f, r)]);
        printf("\n");
    }
    printf("};\n\n");

    printf("constexpr int eg_bishop_table[64] = {\n");
    for (int r=7; r>=0; r--)
    {
        for (int f=0; f<8; f++)
            printf("%5d, ", eg_bishop_table[Square::square(f, r)]);
        printf("\n");
    }
    printf("};\n\n");

    printf("constexpr int mg_rook_table[64] = {\n");
    for (int r=7; r>=0; r--)
    {
        for (int f=0; f<8; f++)
            printf("%5d, ", mg_rook_table[Square::square(f, r)]);
        printf("\n");
    }
    printf("};\n\n");

    printf("constexpr int eg_rook_table[64] = {\n");
    for (int r=7; r>=0; r--)
    {
        for (int f=0; f<8; f++)
            printf("%5d, ", eg_rook_table[Square::square(f, r)]);
        printf("\n");
    }
    printf("};\n\n");

    printf("constexpr int mg_queen_table[64] = {\n");
    for (int r=7; r>=0; r--)
    {
        for (int f=0; f<8; f++)
            printf("%5d, ", mg_queen_table[Square::square(f, r)]);
        printf("\n");
    }
    printf("};\n\n");

    printf("constexpr int eg_queen_table[64] = {\n");
    for (int r=7; r>=0; r--)
    {
        for (int f=0; f<8; f++)
            printf("%5d, ", eg_queen_table[Square::square(f, r)]);
        printf("\n");
    }
    printf("};\n\n");

    printf("constexpr int mg_king_table[64] = {\n");
    for (int r=7; r>=0; r--)
    {
        for (int f=0; f<8; f++)
            printf("%5d, ", mg_king_table[Square::square(f, r)]);
        printf("\n");
    }
    printf("};\n\n");

    printf("constexpr int eg_king_table[64] = {\n");
    for (int r=7; r>=0; r--)
    {
        for (int f=0; f<8; f++)
            printf("%5d, ", eg_king_table[Square::square(f, r)]);
        printf("\n");
    }
    printf("};\n\n");
}
#include <iostream>
#include <fstream>
void printlog(const std::string& message)
{
    std::ofstream myfile;
    std::string str = Home;
    str += "/debug.txt";
      myfile.open(str, std::ios_base::app); // append instead of overwrite
      myfile << message << std::endl;
      myfile.close();
}
