#ifndef PIECE_H
#define PIECE_H

class Piece;

#include <iostream>
#include <string>

#include "defines.h"

// code inspiré de GERBIL

class Piece
{
public:
    Piece();
    Piece(PieceType t, Color c, Square s);

    PieceType   type()   const { return(_type);     }
    Color       color()  const { return(_color);    }
    bool        dead()   const { return(_dead);     }
    Square      square() const { return(_square);   }
    U32         value()  const { return(PIECEVALUES[_type]); }

    bool        isColor(const Color c)                          const { return(c == _color);                }
    bool        isPieceType(const PieceType t)                  const { return(t == _type);                 }
    bool        isColorType(const Color c, const PieceType t)   const { return(c == _color && t == _type);  }

    void setColor(Color c)      { _color  = c;   }
    void setType(PieceType t)   { _type   = t;   }
    void setSquare(Square s)    { _square = s;   }
    void setDead(bool d)        { _dead   = d;   }

    std::string to_string() const;
    char        to_char() const;
    std::string to_name() const;
    friend std::ostream& operator<<(std::ostream& out, const Piece piece);

private:
    PieceType   _type;      // Type of piece.
    Color       _color;     // Its color.
    bool        _dead;      // idée de Gerbil : permet de savoir si une pièce a été capturée
                            // sans avoir à gérer une liste ou autre nonsens

    Square      _square;    //


};

#endif // PIECE_H
