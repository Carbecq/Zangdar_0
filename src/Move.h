#ifndef MOVE_H
#define MOVE_H

class Move;

#include <string>
#include "defines.h"


class Move
{
private:
    I32     _score;     // sera utilisé pour le tri (MVV VLA)
    U32     _code;      // valeur servant pour les Killer Moves; elle définit de façon unique le coup

    static File file(const Square s) {
        return File(s & 7);
    }
    static Rank rank(const Square s) {
        return Rank(s >> 4);
    }


public:
    Move();
    Move(Square from, Square dest, PieceType type, U32 flags);

    Square      from()      const { return( static_cast<Square>(    (_code & CMF_FROM)            ));  }
    Square      dest()      const { return( static_cast<Square>(    (_code & CMF_DEST)      >>  7 ));  }
    PieceType   type()      const { return( static_cast<PieceType>( (_code & CMF_TYPE)      >> 14 ));  }
    PieceType   promotion() const { return( static_cast<PieceType>( (_code & CMF_PROMOTION) >> 17 ));  }

    U32         code()      const { return(_code);      }
    I32         score()     const { return(_score);     }

    void        setScore(I32 v)            { _score = v;       }
    inline void setCode(int from, int dest, int type, int promo, U32 flags) { _code = (from) | (dest<<7) | (type<<14) | (promo<<17) | (flags<<20); }
 //   inline void setCode(U32 code)                                           { _code = code;                                                        }

    bool        none()      const { return(_code & CMF_NONE);         } // ???
    bool        capture()   const { return((_code >> 20) & CMF_CAPTURE   );    }
    bool        pawn2()     const { return((_code >> 20) & CMF_PAWN2    );        }
    bool        pep()       const { return((_code >> 20) & CMF_PEP);          }

    bool        castle_wk() const { return((_code >> 20) & CMF_CASTLE_WK);    }
    bool        castle_wq() const { return((_code >> 20) & CMF_CASTLE_WQ);    }
    bool        castle_bk() const { return((_code >> 20) & CMF_CASTLE_BK);    }
    bool        castle_bq() const { return((_code >> 20) & CMF_CASTLE_BQ);    }

    bool        castle_w() const { return((_code >> 20)  & CMF_CASTLE_W);    }
    bool        castle_b() const { return((_code >> 20)  & CMF_CASTLE_B);    }
    bool        castle()   const { return((_code >> 20)  & CMF_CASTLE);      }



    std::string show(int mode=0) const;
    bool        equal(const std::string & str);



};

#endif // MOVE_H
