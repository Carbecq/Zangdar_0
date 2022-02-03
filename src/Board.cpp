#include "defines.h"
#include "Board.h"


Board::Board()
{
#ifdef DEBUG_CLASS
    std::cout << "Board::Board" << std::endl;
#endif

    // De façon à éviter le test : board[Square] == nullptr
    // on va allouer une pièce pour chaque case

    for (int i=0; i<BOARD_SIZE; i++)
    {
        vide[i]  = Piece(EMPTY, WHITE, static_cast<Square>(i));
    }

    positions = new Position();
    zobrist   = new Zobrist();

    init();
    //    flip();
}

Board::~Board()
{
#ifdef DEBUG_CLASS
    std::cout << "Board::~Board" << std::endl;
#endif

    delete zobrist;
    delete positions;
}

void Board::init()
{
#ifdef DEBUG_CLASS
    std::cout << "Board::init" << std::endl;
#endif

    output      = 4;

    for (int i=0; i<BOARD_SIZE; i++)
    {
        board[i] = &vide[i];
    }

    for (auto & e : moves)
    {
        e = Move();
    }

    init_attack();
    tests();
}
void Board::reset()
{
#ifdef DEBUG_CLASS
    std::cout << "Board::reset" << std::endl;
#endif

    // Il y a une pièce sur chaque case, de façon à éviter le test (== nullptr)

    for (int i=0; i<BOARD_SIZE; i++)
    {
        board[i] = &vide[i];
    }

    for (auto& e: first_move)
        e = 0;

    // Reset the piece lists
    pieces[0].clear();
    pieces[1].clear();

    positions->reset();
}

void Board::tests()
{
    for (Rank r : RANKS)
    {
        for (File f : FILES)
        {
            Square s = square(r, f);

            Rank rr = rank(s);
            File ff = file(s);

            if (r != rr)
                std::cout << "r error" << std::endl;
            if (f != ff)
                std::cout << "f error" << std::endl;

            //            std::cout << "r=" << r << " f=" << f << " s=" << s << " r=" << rr << " f=" << ff << std::endl;
        }
    }
}

//===========================================================================
// Regarde dans la liste des coups si le coup de la Principal Variation
// existe. Dans ce cas, on donne à ce coup un gros bonus de façon à ce
// qu'il soit joué en premier par la fonction de recherche.
//----------------------------------------------------------------------------
void Board::pv_move(U32 PvMove, int ply)
{
    // https://www.chessprogramming.org/Principal_Variation

    for (int index = first_move[ply]; index < first_move[ply + 1]; ++index)
    {
        if( moves[index].code() == PvMove)
        {
            moves[index].setScore( 2000000 );
            break;
        }
    }
}

int Board::nbr_pieces(Color side) const
{
    int total = 0;
    for (auto & p : pieces[side])
    {
        if (p.dead() == false)
        {
            switch(p.type())
            {
            case PAWN:
                break;
            case KNIGHT:
                total++;
                break;
            case BISHOP:
                total++;
                break;
            case ROOK:
                total++;
                break;
            case QUEEN:
                total++;
                break;
            case KING:
                break;
            default:
                break;
            }
        }
    }

    return(total);
}
