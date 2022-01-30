#include <cassert>

#include "defines.h"
#include "Board.h"

static int MvvLvaScores[7][7];


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
void Board::gen_moves()
{
    int k = first_move[positions->ply];

    if (positions->side_to_move == WHITE)
        gen_wmoves(k);
    else
        gen_bmoves(k);

    first_move[positions->ply + 1] = k;
}

//=================================================================
//! \brief  Génération des coups pour les Blancs
//!
//! \param[in]  pos    position dans laquelle on fait la génération
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_wmoves(int& index)
{
    Square s;

    for (auto& p : pieces[WHITE])
    {
        if (p.dead() == true)
            continue;

        s = p.square();
        assert(s>=A1 && s <= H8);
        assert(rank(s)>=RANK_1 && rank(s)<=RANK_8);
        assert(file(s)>=FILE_A && file(s)<=FILE_H);

        switch(p.type())
        {
        case PAWN:
            gen_wpawn(s, index);
            break;
        case KNIGHT:
            gen_knight(WHITE, s, index);
            break;
        case BISHOP:
            gen_bishop(WHITE, s, index);
            break;
        case ROOK:
            gen_rook(WHITE, s, index);
            break;
        case QUEEN:
            gen_queen(WHITE, s, index);
            break;
        case KING:
            gen_wking(s, index);
            break;
        default:
            break;
        }
    }

    //        std::cout << "total WHITE = " << index - pos->first_move << std::endl;

    //        for (int i=pos->first_move; i<index; i++)
    //        {
    //            std::cout << i << "  " << moves[i].show() << std::endl;
    //        }
}

//=================================================================
//! \brief  Génération des coups pour les Noirs
//!
//! \param[in]  pos    position dans laquelle on fait la génération
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_bmoves(int& index)
{
    Square s;

    for (auto& p : pieces[BLACK])
    {
        if (p.dead() == true)
            continue;

        s = p.square();
        assert(s>=A1 && s <= H8);
        assert(rank(s)>=0 && rank(s)<=7);
        assert(file(s)>=0 && file(s)<=7);

        switch(p.type())
        {
        case PAWN:
            gen_bpawn(s, index);
            break;
        case KNIGHT:
            gen_knight(BLACK, s, index);
            break;
        case BISHOP:
            gen_bishop(BLACK, s, index);
            break;
        case ROOK:
            gen_rook(BLACK, s, index);
            break;
        case QUEEN:
            gen_queen(BLACK, s, index);
            break;
        case KING:
            gen_bking(s, index);
            break;
        default:
            break;
        }
    }

    //        std::cout << "total BLACK = " << index - pos->first_move << std::endl;

    //        for (int i=pos->first_move; i<index; i++)
    //        {
    //            std::cout << i << "  " << moves[i].show() << std::endl;
    //        }

}

//=================================================================
//! \brief  Génération des coups pour un Pion Blanc
//!
//! \param[in]  pos    position dans laquelle on fait la génération
//! \param[in]  from    position du pion
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_wpawn(Square from, int& index)
{
    Square  to;
    U32    flags = CMF_NONE;

    /* Avance d'une ou de deux cases. */

    to = static_cast<Square>(from + 16);

    if (!(to & 0x88) && board[to]->type() == EMPTY)
    {
        if (rank(to) == RANK_8)
        {
            // promotions : tour, cavalier, fou, dame

            add_quiet_promote(from, to, flags, index);
        }
        else if (rank(from) == RANK_2)
        {
      //       std::cout << "avance wp debut 1" << std::endl;
            add_quiet_move(from, to, EMPTY, flags, index);

            // Avance de 2 cases

            to = static_cast<Square>(to + 16);
            if (!(to & 0x88) && board[to]->type() == EMPTY)
            {
      //          std::cout << "avance wp 2" << std::endl;
                add_quiet_move(from, to, EMPTY, flags | CMF_PAWN2, index);
            }
        }
        else
        {
     //      std::cout << "avance wp normal 1" << std::endl;
            add_quiet_move(from, to, EMPTY, flags, index);
        }
    }

    /* Prises. */

    flags = CMF_CAPTURE;

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
                    add_capture_promote(from, to, flags, index);
                }
                else
                {
                    //                    std::cout << "prise wp 1" << std::endl;
                    add_capture_move(from, to, EMPTY, flags, index);
                }
            }
            else if (to == positions->ep_square && rank(to) == RANK_6)
            {
                // prise en passant
                add_enpassant_move(from, to, flags | CMF_PEP, index);
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
void Board::gen_bpawn(Square from, int& index)
{
    Square to;
    U32 flags = CMF_NONE;

    /* Avance d'une ou de deux cases. */

    to = static_cast<Square>(from - 16);

    if (!(to & 0x88) && board[to]->type() == EMPTY)
    {
        if (rank(to) == RANK_1)
        {
            // promotions : tour, cavalier, fou, dame

            add_quiet_promote(from, to, flags, index);
        }
        else if (rank(from) == RANK_7)
        {
            //           std::cout << "avance bp debut 1" << std::endl;
            add_quiet_move(from, to, EMPTY, flags, index);

            // Avance de 2 cases

            to = static_cast<Square>(to - 16);
            if (!(to & 0x88) && board[to]->type() == EMPTY)
            {
                add_quiet_move(from, to, EMPTY, flags | CMF_PAWN2, index);
            }
        }
        else
        {
            //           std::cout << "avance bp normal 1" << std::endl;
            add_quiet_move(from, to, EMPTY, flags, index);
        }
    }

    /* Prises. */

    flags = CMF_CAPTURE;

    for (int i=0; i<2; i++)
    {
        to = static_cast<Square>(from + delta_bpawn[i]);

        if (!(to & 0x88))
        {
            if (board[to]->type() != EMPTY && board[to]->color() == WHITE)
            {
                // promotions : tour, cavalier, fou, dame
                if (rank(to) == RANK_1)
                {
                    add_capture_promote(from, to, flags, index);
                }
                else
                {
                    //                   std::cout << "prise bp 1" << std::endl;
                    add_capture_move(from, to, EMPTY, flags, index);
                }
            }
            else if (to == positions->ep_square && rank(to) == RANK_3)
            {
                // prise en passant
                add_enpassant_move(from, to, flags | CMF_PEP, index);
            }
        }
    }

    //       std::cout << "total bpawn " << ascii(from) << " = " << total << std::endl;
}

//=================================================================
//! \brief  Génération des coups pour un Cavalier
//!
//! \param[in]  c       camp du cavalier
//! \param[in]  from    position du cavalier
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_knight(Color c, Square from, int& index)
{
    //    std::cout << "gen knight from = " << from << std::endl;
    Square to;
    U32 flags = CMF_NONE;

    for (int i=0; i<8; i++)
    {
        to = static_cast<Square>(from + delta_knight[i]);

        if (!(to & 0x88))			// Is the destination Square on the board?
        {
            if (board[to]->type() == EMPTY)
            {
                add_quiet_move(from, to, EMPTY, flags, index);
            }
            else if (board[to]->color() != c)
            {
                add_capture_move(from, to, EMPTY, flags | CMF_CAPTURE, index);
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
void Board::gen_wking(Square from, int& index)
{
    //    std::cout << "gen wking from = " << from << std::endl;
    Square to;
    U32 flags = CMF_NONE;

    for (int i=0; i<8; i++)
    {
        to = static_cast<Square>(from + delta_king[i]);

        if (!(to & 0x88))			// Is the destination Square on the board?
        {
            if (board[to]->type() == EMPTY)
            {
                add_quiet_move(from, to, EMPTY, flags, index);
            }
            else if (board[to]->color() == BLACK)
            {
                add_capture_move(from, to, EMPTY, flags | CMF_CAPTURE, index);
            }
        }
    }

    // conditions du roque :
    //--------------------------------------------------------------
    // le roi ni la tour ne doivent avoir bougé (flag castle)
    // le roi n'est pas en échec
    // la case d'arrivée du roi n'est pas en échec
    // les cases de déplacement du roi ne sont pas en échec
    // les cases entre le roi et la tour sont vides


    if((positions->castle & CASTLE_WQ)  &&
            board[B1]->type() == EMPTY &&
            board[C1]->type() == EMPTY &&
            board[D1]->type() == EMPTY &&
            !is_attacked_by(E1, BLACK) &&  // on n'est pas en échec
            !is_attacked_by(C1, BLACK) &&
            !is_attacked_by(D1, BLACK) )
    {
        add_quiet_move(E1, C1, EMPTY, flags | CMF_CASTLE_WQ, index);
    }

    if((positions->castle & CASTLE_WK)  &&
            board[F1]->type() == EMPTY &&
            board[G1]->type() == EMPTY &&
            !is_attacked_by(E1, BLACK) &&  // on n'est pas en échec
            !is_attacked_by(F1, BLACK) &&
            !is_attacked_by(G1, BLACK))
    {
        add_quiet_move(E1, G1, EMPTY, flags | CMF_CASTLE_WK, index);
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
void Board::gen_bking(Square from, int& index)
{
    Square to;
    U32 flags = CMF_NONE;

    for (int i=0; i<8; i++)
    {
        to = static_cast<Square>(from + delta_king[i]);

        if (!(to & 0x88))			// Is the destination Square on the board?
        {
            if (board[to]->type() == EMPTY)
            {
                add_quiet_move(from, to, EMPTY, flags, index);
            }
            else if (board[to]->color() == WHITE)
            {
                add_capture_move(from, to, EMPTY, flags | CMF_CAPTURE, index);
            }
        }
    }

    // conditions du roque :
    //--------------------------------------------------------------
    // le roi ni la tour ne doivent avoir bougé (flag castle)
    // le roi n'est pas en échec
    // la case d'arrivée du roi n'est pas en échec
    // les cases de déplacement du roi ne sont pas en échec
    // les cases entre le roi et la tour sont vides

    if((positions->castle & CASTLE_BQ)  &&
            board[B8]->type() == EMPTY &&
            board[C8]->type() == EMPTY &&
            board[D8]->type() == EMPTY &&
            !is_attacked_by(E8, WHITE) &&  // on n'est pas en échec
            !is_attacked_by(C8, WHITE) &&
            !is_attacked_by(D8, WHITE))
    {
        add_quiet_move(E8, C8, EMPTY, flags | CMF_CASTLE_BQ, index);
    }

    if((positions->castle & CASTLE_BK)  &&
            board[F8]->type() == EMPTY &&
            board[G8]->type() == EMPTY &&
            !is_attacked_by(E8, WHITE) &&  // on n'est pas en échec
            !is_attacked_by(F8, WHITE) &&
            !is_attacked_by(G8, WHITE))
    {
        add_quiet_move(E8, G8, EMPTY, flags | CMF_CASTLE_BK, index);
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
void Board::gen_rook(Color c, Square from, int& index)
{
    //    std::cout << "rook : ";
    gen_slider(c, from, delta_rook, index);
}

//=================================================================
//! \brief  Génération des coups pour un Fou
//!
//! \param[in]  c       camp du fou
//! \param[in]  from    position du fou
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_bishop(Color c, Square from, int& index)
{
    //    std::cout << "rook : ";
    gen_slider(c, from, delta_bishop, index);
}

//=================================================================
//! \brief  Génération des coups pour une Dame
//!
//! \param[in]  c       camp de la dame
//! \param[in]  from    position de la dame
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::gen_queen(Color c, Square from, int& index)
{
    //    std::cout << "queen : ";
    gen_slider(c, from, delta_rook, index);
    gen_slider(c, from, delta_bishop, index);
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
void Board::gen_slider(Color c, Square from, const int delta[4], int& index)
{
    Square  to;
    U32    flags = CMF_NONE;

    //    std::cout << "gen slider from = " << from << std::endl;

    for (int i=0; i<4; i++) 	// pour chaque direction de recherche
    {
        to = static_cast<Square>(from + delta[i]);

        while (!(to & 0x88) && (board[to]->type() == EMPTY))
        {
            add_quiet_move(from, to, EMPTY, flags, index);
            to = static_cast<Square>(to + delta[i]);
        }
        if (!(to & 0x88) && board[to]->color() != c)
        {
            add_capture_move(from, to, EMPTY, flags | CMF_CAPTURE, index);
        }
    }

    //   std::cout << "total slider = " << total << std::endl;
}

//=================================================================
//! \brief  Ajout d'un coup tranquille à la liste des coups
//!
//! \param[in]  from    position de départ de la pièce
//! \param[in]  to      position d'arrivée de la pièce
//! \param[in]  flags   flags déterminant le type du coup
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::add_quiet_move(Square from, Square to, PieceType promo, U32 flags, int& index)
{
    moves[index].setCode(from, to, board[from]->type(), promo, flags);

//    U32 code = moves[index].code();
//   string binary = std::bitset<32>(code).to_string(); //to binary
//        std::cout<<"recup   " << binary<<"\n";


    if (searchKillers[0][positions->ply] == moves[index].code())
        moves[index].setScore(900000);
    else if (searchKillers[1][positions->ply] == moves[index].code())
        moves[index].setScore(800000);
    else
        moves[index].setScore( searchHistory[positions->side_to_move][moves[index].type()][to] );

    index++;
}

//=================================================================
//! \brief  Ajout d'un coup de capture à la liste des coups
//!
//! \param[in]  from    position de départ de la pièce
//! \param[in]  to      position d'arrivée de la pièce
//! \param[in]  flags   flags déterminant le type du coup
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::add_capture_move(Square from, Square to, PieceType promo, U32 flags, int& index)
{
    moves[index].setCode(from, to, board[from]->type(), promo, flags);

    //        printf("cap vic=%d att=%d score=%d \n", board[to]->type(), board[from]->type(),
    //              MvvLvaScores[board[to]->type()][board[from]->type()] );
    moves[index].setScore( MvvLvaScores[board[to]->type()][board[from]->type()] + 1000000);

    // list->moves[list->count].score = MvvLvaScores[CAPTURED(move)][pos->pieces[FROMSQ(move)]];

    // MvvLvaScores[Victim][Attacker]

    index++;
}

//=================================================================
//! \brief  Ajout d'un coup de prise en passant à la liste des coups
//!
//! l'échiquier ne contient aucune pièce dans la case 'to'
//! la routine add_capture_move ne peut pas fonctionner
//!
//! \param[in]  from    position de départ de la pièce
//! \param[in]  to      position d'arrivée de la pièce
//! \param[in]  flags   flags déterminant le type du coup
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::add_enpassant_move(Square from, Square to, U32 flags, int& index)
{
    moves[index].setCode(from, to, PAWN, EMPTY, flags);
    moves[index].setScore( MvvLvaScores[PAWN][PAWN] + 1000000);

    index++;
}


//=================================================================
//! \brief  Ajout des coups pour une promotion
//!
//! \param[in]  from    position de départ de la pièce
//! \param[in]  to      position d'arrivée de la pièce
//! \param[in]  flags   flags déterminant le type du coup
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::add_quiet_promote(Square from, Square to, U32 flags, int& index)
{
    add_quiet_move(from, to, KNIGHT, flags, index);
    add_quiet_move(from, to, BISHOP, flags, index);
    add_quiet_move(from, to, ROOK,   flags, index);
    add_quiet_move(from, to, QUEEN,  flags, index);
}

//=================================================================
//! \brief  Ajout des coups pour une promotion
//!
//! \param[in]  from    position de départ de la pièce
//! \param[in]  to      position d'arrivée de la pièce
//! \param[in]  flags   flags déterminant le type du coup
//! \param[out] index   index dans la pile des coups à partir duquel
//!                     on va ajouter les coups
//-----------------------------------------------------------------
void Board::add_capture_promote(Square from, Square to, U32 flags, int& index)
{
    add_capture_move(from, to, KNIGHT, flags, index);
    add_capture_move(from, to, BISHOP, flags, index);
    add_capture_move(from, to, ROOK,   flags, index);
    add_capture_move(from, to, QUEEN,  flags, index);
}

//=============================================================
//! \brief Teste si le camp c est en échec
//! \param[in] c    couleur du camp
//-------------------------------------------------------------
bool Board::in_check(Color c)
{
//     fprintf(fp, "in_check debut \n");fflush(fp);

//     fprintf(fp, "in_check s=%d c=%d \n", pieces[c][0].square(), c);fflush(fp);

    return(is_attacked_by(pieces[c][0].square(), static_cast<Color>(!c) ));
}

// const int VictimScore[7] = { PieceValue[PAWN], 100, 200, 300, 400, 500, 600}; //, 100, 200, 300, 400, 500, 600 };

// https://www.chessprogramming.org/MVV-LVA
// MVV/LVA=Most Valuable Victim/Least Valuable Attacker,
// https://open-chess.org/viewtopic.php?t=3058
// https://stackoverflow.com/questions/37878665/quiescence-search-in-a-chess-computer
// https://www.stmintz.com/ccc/index.php?id=45677
// https://www.stmintz.com/ccc/index.php?id=45655

//==========================================================================
//! \brief  Initialisation du score pour le tri des coups
//--------------------------------------------------------------------------
void Board::init_MVVLVA(void)
{
    for(int Attacker = PAWN; Attacker <= KING; ++Attacker) {
        for(int Victim = PAWN; Victim <= KING; ++Victim) {
            //           MvvLvaScores[Victim][Attacker] = VictimScore[Victim] + 6 - ( VictimScore[Attacker] / 100);
            //           MvvLvaScores[Victim][Attacker] = PIECEVALUES[Victim] + 6 - (PIECEVALUES[Attacker] / 100);
            MvvLvaScores[Victim][Attacker] = PIECEVALUES[Victim] + 6 - Attacker;
        }
    }

    //    for(int Attacker = PAWN; Attacker <= KING; ++Attacker) {
    //        printf("att=%d ", Attacker);
    //        for(int Victim = PAWN; Victim <= KING; ++Victim) {
    //            printf("vic=%d score=%d ", Victim, MvvLvaScores[Victim][Attacker]);
    //        }
    //        printf("\n");
    //    }
}
