#include "defines.h"
#include "Board.h"

// On ne génère ici que les captures et les promotions

//=================================================================
//! \brief  Génération des coups pour une position donnée
//!
//!  moves[MAX_MOVES] est une pile de stockage des coups
//!
//!  +--------+------+----
//!  A        B      C
//!
//!  A = index à partir duquel les coups à une profondeur de 0 seront stockés
//!  B = ""                                                  1   "
//!
//!  à chaque profondeur correspond une position, qui est stockée
//!  dans le tableau positions
//!
//!  chaque position contient l'élément first_move qui donne l'index
//!  de stockage des coups
//!
//! \param[in] pos position dans laquelle on fait la génération
//!
//-----------------------------------------------------------------
void Board::gen_caps()
{
    int index = first_move[positions->ply];

    if (positions->side_to_move == WHITE)
        gen_wcaps(index);
    else
        gen_bcaps(index);

    first_move[positions->ply + 1] = index;
}

//=================================================================
//! \brief  Génération des coups pour les Blancs
//!
//! \param[in]  pos     position dans laquelle on fait la génération
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_wcaps(int& index)
{
    Square s;

    for (auto& p : pieces[WHITE])
    {
        if (p.dead() == true)
            continue;

        s = p.square();

        switch(p.type())
        {
        case PAWN:
            gen_wpawn_caps(s, index);
            break;
        case KNIGHT:
            gen_knight_caps(WHITE, s, index);
            break;
        case BISHOP:
            gen_bishop_caps(WHITE, s, index);
            break;
        case ROOK:
            gen_rook_caps(WHITE, s, index);
            break;
        case QUEEN:
            gen_queen_caps(WHITE, s, index);
            break;
        case KING:
            gen_wking_caps(s, index);
            break;
        default:
            break;
        }
    }

    //    std::cout << "total WHITE = " << index - pos->first_move << std::endl;

    //    for (int i=pos->first_move; i<index; i++)
    //    {
    //        std::cout << i << "  " << moves[i].show() << std::endl;
    //    }
}

//=================================================================
//! \brief  Génération des coups pour les Noirs
//!
//! \param[in]  pos    position dans laquelle on fait la génération
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_bcaps(int& index)
{
    Square s;

    for (auto& p : pieces[BLACK])
    {
        if (p.dead() == true)
            continue;

        s = p.square();

        switch(p.type())
        {
        case PAWN:
            gen_bpawn_caps(s, index);
            break;
        case KNIGHT:
            gen_knight_caps(BLACK, s, index);
            break;
        case BISHOP:
            gen_bishop_caps(BLACK, s, index);
            break;
        case ROOK:
            gen_rook_caps(BLACK, s, index);
            break;
        case QUEEN:
            gen_queen_caps(BLACK, s, index);
            break;
        case KING:
            gen_bking_caps(s, index);
            break;
        default:
            break;
        }
    }

    //    std::cout << "total BLACK = " << index - pos->first_move << std::endl;

    //    for (int i=pos->first_move; i<index; i++)
    //    {
    //        std::cout << i << "  " << moves[i].show() << std::endl;
    //    }

}

//=================================================================
//! \brief  Génération des coups pour un Pion Blanc
//!
//! \param[in]  pos    position dans laquelle on fait la génération
//! \param[in]  from    position du pion
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_wpawn_caps(Square from, int& index)
{
    Square  to;

    /* Avance de deux cases. */

    to = static_cast<Square>(from + 16);

    if (!(to & 0x88) && board[to]->type() == EMPTY)
    {
        if (rank(to) == RANK_8)
        {
            // promotions : tour, cavalier, fou, dame

            add_quiet_promote(from, to, CMF_NONE, index);
        }
    }

    /* Prises. */

    for (int i=0; i<2; i++)
    {
        to = static_cast<Square>(from + delta_wpawn[i]);

        if (!(to & 0x88))
        {
            if (board[to]->type() != EMPTY && board[to]->color() == BLACK)
            {
                // promotions : tour, cavalier, fou, dame
                if (rank(to) == RANK_8)
                {
                    add_capture_promote(from, to, CMF_CAPTURE, index);
                }
                else
                {
                    //                    std::cout << "prise wp 1" << std::endl;
                    add_capture_move(from, to, EMPTY, CMF_CAPTURE, index);
                }
            }
            else if (to == positions->ep_square && rank(to) == RANK_6)
            {
                // prise en passant
                add_capture_move(from, to, EMPTY, CMF_CAPTURE | CMF_PEP, index);
            }
        }
    }

    //    std::cout << "total wpawn = " << total << std::endl;
}

//=================================================================
//! \brief  Génération des coups pour un Pion Noir
//!
//! \param[in]  pos    position dans laquelle on fait la génération
//! \param[in]  from    position du pion
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_bpawn_caps(Square from, int& index)
{
    Square to;

    /* Avance d'une ou de deux cases. */

    to = static_cast<Square>(from - 16);

    if (!(to & 0x88) && board[to]->type() == EMPTY)
    {
        if (rank(to) == RANK_1)
        {
            // promotions : tour, cavalier, fou, dame

            add_quiet_promote(from, to, CMF_NONE, index);
        }
    }

    /* Prises. */

    for (int i=0; i<2; i++)
    {
        to = static_cast<Square>(from + delta_bpawn[i]);

        if (!(to & 0x88))
        {
            //         printf("f=%s to=%s t=%d c=%d \n", ascii(from).c_str(), ascii(to).c_str(), board[to]->type(), board[to]->color());
            if (board[to]->type() != EMPTY && board[to]->color() == WHITE)
            {
                // promotions : tour, cavalier, fou, dame
                if (rank(to) == RANK_1)
                {
                    add_capture_promote(from, to, CMF_CAPTURE, index);
                }
                else
                {
                    //                   std::cout << "prise wp 1" << std::endl;
                    add_capture_move(from, to, EMPTY, CMF_CAPTURE, index);
                }
            }
            else if (to == positions->ep_square && rank(to) == RANK_3)
            {
                // prise en passant
                add_capture_move(from, to, EMPTY, CMF_CAPTURE | CMF_PEP, index);
            }
        }
    }

    //    std::cout << "total bpawn " << ascii(from) << " = " << total << std::endl;
}

//=================================================================
//! \brief  Génération des coups pour un Cavalier
//!
//! \param[in]  c       camp du cavalier
//! \param[in]  from    position du cavalier
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_knight_caps(Color c, Square from, int& index)
{
    //    std::cout << "gen knight from = " << from << std::endl;
    Square to;

    for (int i=0; i<8; i++)
    {
        to = static_cast<Square>(from + delta_knight[i]);

        if (!(to & 0x88))			// Is the destination Square on the board?
        {
            if (board[to]->type() != EMPTY && board[to]->color() != c)
            {
                add_capture_move(from, to, EMPTY, CMF_CAPTURE, index);
            }
        }
    }

    //    std::cout << "total knight = " << total << std::endl;
}

//=================================================================
//! \brief  Génération des coups pour un Roi Blanc
//!
//! \param[in]  pos    position dans laquelle on fait la génération
//! \param[in]  from    position du roi
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_wking_caps(Square from, int& index)
{
    //    std::cout << "gen wking from = " << from << std::endl;
    Square to;

    for (int i=0; i<8; i++)
    {
        to = static_cast<Square>(from + delta_king[i]);

        if (!(to & 0x88))			// Is the destination Square on the board?
        {
            if (board[to]->type() != EMPTY && board[to]->color() == BLACK)
            {
                add_capture_move(from, to, EMPTY, CMF_CAPTURE, index);
            }
        }
    }

    //    std::cout << "total wking = " << total << std::endl;
}

//=================================================================
//! \brief  Génération des coups pour un Roi Noir
//!
//! \param[in]  pos    position dans laquelle on fait la génération
//! \param[in]  from    position du roi
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_bking_caps(Square from, int& index)
{
    Square to;

    for (int i=0; i<8; i++)
    {
        to = static_cast<Square>(from + delta_king[i]);

        if (!(to & 0x88))			// Is the destination Square on the board?
        {
            if (board[to]->type() != EMPTY && board[to]->color() == WHITE)
            {
                add_capture_move(from, to, EMPTY, CMF_CAPTURE, index);
            }
        }
    }

    //    std::cout << "total bking = " << total << std::endl;
}

//=================================================================
//! \brief  Génération des coups pour une Tour
//!
//! \param[in]  c       camp de la tour
//! \param[in]  from    position de la tour
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_rook_caps(Color c, Square from, int& index)
{
    //    std::cout << "rook : ";
    gen_slider_caps(c, from, delta_rook, index);
}

//=================================================================
//! \brief  Génération des coups pour un Fou
//!
//! \param[in]  c       camp du fou
//! \param[in]  from    position du fou
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_bishop_caps(Color c, Square from, int& index)
{
    //    std::cout << "rook : ";
    gen_slider_caps(c, from, delta_bishop, index);
}

//=================================================================
//! \brief  Génération des coups pour une Dame
//!
//! \param[in]  c       camp de la dame
//! \param[in]  from    position de la dame
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_queen_caps(Color c, Square from, int& index)
{
    //    std::cout << "queen : ";
    gen_slider_caps(c, from, delta_rook, index);
    gen_slider_caps(c, from, delta_bishop, index);
}

//=================================================================
//! \brief  Génération des coups pour une pièce glissante
//!
//! \param[in]  c       camp de la pièce
//! \param[in]  from    position de la pièce
//! \param[in]  delta   tableau des déplacements possibles
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_slider_caps(Color c, Square from, const int delta[4], int& index)
{
    Square  to;

    //    std::cout << "gen slider from = " << from << std::endl;

    for (int i=0; i<4; i++) 	// pour chaque direction de recherche
    {
        to = static_cast<Square>(from + delta[i]);

        while (!(to & 0x88) && (board[to]->type() == EMPTY))
        {
            to = static_cast<Square>(to + delta[i]);
        }
        if (!(to & 0x88) && board[to]->color() != c)
        {
            add_capture_move(from, to, EMPTY, CMF_CAPTURE, index);
        }
    }

    //   std::cout << "total slider = " << total << std::endl;
}

