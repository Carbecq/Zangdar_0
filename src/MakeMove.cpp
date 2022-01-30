#include <cassert>
#include <bitset>

#include "defines.h"
#include "Board.h"


//=========================================================
//! \brief  Effectue un coup sur l'échiquier
//! \param[in] pos      position dans laquelle est effectué le coup
//! \param[in] move     coup à effectuer
//---------------------------------------------------------
bool Board::make_move(const Move* move)
{
    assert(move != nullptr);

    Square from = move->from();
    assert (from>=A1 && from<=H8);

    Square dest = move->dest();
    assert (dest>=A1 && dest<=H8);

    assert(board[from]);

    Color       side = positions->side_to_move;

    // Sauvegarde des caractéristiques de la position
    history[positions->hply].ep_square  = positions->ep_square;
    history[positions->hply].ep_took    = positions->ep_took;
    history[positions->hply].castle     = positions->castle;
    history[positions->hply].captured   = nullptr;
    history[positions->hply].fifty      = positions->fifty;
    history[positions->hply].hash       = positions->hash;
    // Sauvegarde effectuée


    // La prise en passant n'est valable que tout de suite
    // Il faut donc la supprimer

    // hash enpassant if available (remove enpassant square from hash key )
    if (positions->ep_square != OFFBOARD)
        zobrist->update_en_passant(positions->hash, positions->ep_square);

    // reset enpassant square
    positions->ep_square = OFFBOARD;

    // Droit au roque, remove ancient value
    zobrist->update_castle(positions->hash, positions->castle);

    //TODO peut-on le mettre ailleurs , de façon à ne le faire que si c'est utile ?
    // update castling rights
    positions->castle    = positions->castle & castle_mask[from] & castle_mask[dest];

    // Droit au roque; add new value
    zobrist->update_castle(positions->hash, positions->castle);

    // Règle des 50 coups
    if (board[from]->type()==PAWN || move->capture())
        positions->fifty = 0;
    else
        positions->fifty =  positions->fifty + 1;

    // Roque
    if (move->castle())
    {
        //  On effectue le déplacement de la tour
        //  Celui du roi sera effectué avec les coups normaux

        //  si on est ici, c'est que :
        //  + on n'est pas échec
        //  + le roi et la tour sont sur leur case de départ
        //    et n'ont pas bougé
        //  + les cases les cases entre le roi et la tour,
        //    ainsi que la case d'arrivée du roi sont vides
        //  + le roi ne passe pas sur une case en échec

        switch(dest)
        {
        case G1:                        // petit roque blanc
            board[F1] = board[H1];
            board[F1]->setSquare(F1);
            board[H1] = &(vide[H1]);

            zobrist->update_piece(positions->hash, side, ROOK, H1); // remove rook from h1 from hash key
            zobrist->update_piece(positions->hash, side, ROOK, F1); // put rook on f1 into a hash key
            break;

        case C1:                        // grand roque blanc
            board[D1] = board[A1];
            board[D1]->setSquare(D1);
            board[A1] = &(vide[A1]);

            zobrist->update_piece(positions->hash, side, ROOK, A1); // remove rook from a1 from hash key
            zobrist->update_piece(positions->hash, side, ROOK, D1); // put rook on d1 into a hash key
            break;

        case C8:                        // petit roque noir
            board[D8] = board[A8];
            board[D8]->setSquare(D8);
            board[A8] = &(vide[A8]);

            zobrist->update_piece(positions->hash, side, ROOK, A8); //
            zobrist->update_piece(positions->hash, side, ROOK, D8); //
            break;

        case G8:                        // grand roque noir
            board[F8] = board[H8];
            board[F8]->setSquare(F8);
            board[H8] = &(vide[H8]);

            zobrist->update_piece(positions->hash, side, ROOK, H8); //
            zobrist->update_piece(positions->hash, side, ROOK, F8); //
            break;
        default:
            break;
        }

        board[dest] = board[from];             // déplacement du roi
        board[dest]->setSquare(dest);
        board[from] = &(vide[from]);

        zobrist->update_piece(positions->hash, side, KING, from); // remove KING from source square in hash key
        zobrist->update_piece(positions->hash, side, KING, dest); // set KING to the target square in hash key
    }

    // Case de prise en passant  : on a joué e2-e4
    else if (move->pawn2())
    {
        //       printf("side=%d makemove pawn2       : f=%d d=%d t=%d \n", pos->side_to_move, from, dest, board[from]->type());

        board[dest] = board[from];             // déplacement du pion
        board[dest]->setSquare(dest);
        board[from] = &(vide[from]);

        if (side == WHITE)
            positions->ep_square = static_cast<Square>(dest - 16);
        else
            positions->ep_square = static_cast<Square>(dest + 16);

        positions->ep_took = dest;

        zobrist->update_piece(positions->hash, side, PAWN, from); // enlève le pion E2
        zobrist->update_piece(positions->hash, side, PAWN, dest); // ajoute le pion E4

        zobrist->update_en_passant(positions->hash, positions->ep_square);  // met à jour la case en_passant
    }

    // Capture
    else if (move->capture())
    {
         Color     xside = static_cast<Color>(!side);
                 // ~side; // static_cast<Color>(!side);

        //      printf("side=%d makemove capture     : f=%d d=%d t=%d \n", pos->side_to_move, from, dest, board[from]->type());

        if (move->pep())                            // prise en passant
        {
            history[positions->hply].captured = board[positions->ep_took];      // sauvegarde du pion pris
            board[positions->ep_took]->setDead(true);                           // pion adverse pris
            board[positions->ep_took] = &(vide[positions->ep_took]);            // pion capturé

            board[dest] = board[from];                                          // déplacement du pion prenant
            board[dest]->setSquare(dest);
            board[from] = &(vide[from]);                                        // le pion qui prend

            zobrist->update_piece(positions->hash, side, PAWN, from);                   // remove PAWN from source square in hash key
            zobrist->update_piece(positions->hash, side, PAWN, dest);                   // set PAWN to the target square in hash key
            zobrist->update_piece(positions->hash, xside, PAWN, positions->ep_took);    // remove taken PAWN from hash key
        }
        else
        {
            PieceType   type  = board[from]->type();    // pièce prenante
            PieceType   xtype = board[dest]->type();    // pièce prise

            history[positions->hply].captured = board[dest];           // sauvegarde de la pièce prise
            assert(board[dest]);
            board[dest]->setDead(true);

            board[dest] = board[from];              // déplacement de la pièce prenant
            board[dest]->setSquare(dest);
            board[from] = &(vide[from]);

            zobrist->update_piece(positions->hash,  side,  type, from);     // enlève la pièce prenante de sa case de départ
            zobrist->update_piece(positions->hash, xside, xtype, dest);     // enlève la pièce prise de sa case

            if (move->promotion())
            {
                board[dest]->setType(move->promotion());

                zobrist->update_piece(positions->hash, side, board[dest]->type(), dest); // ajoute la pièce promue, 'board[dest]->type' a changé !!
            }
            else
            {
                zobrist->update_piece(positions->hash, side, type, dest); // déplace la pièce prenante
            }
        }
    }

    // Déplacement
    else
    {
        //         printf("side=%d makemove déplacement : f=%d d=%d t=%d \n", positions->side_to_move, from, dest, board[from]->type());
        PieceType   type  = board[from]->type();

        board[dest] = board[from];             // déplacement de la pièce se déplaçant
        board[dest]->setSquare(dest);
        board[from] = &(vide[from]);

        zobrist->update_piece(positions->hash, side, type, from); // remove piece from source square in hash key

        if (move->promotion())
        {
            board[dest]->setType(move->promotion());
            zobrist->update_piece(positions->hash, side, board[dest]->type(), dest);
        }
        else
        {
            zobrist->update_piece(positions->hash, side, type, dest); // set piece to the target square in hash key
        }
    }


    /* switch sides and test for legality (if we can capture
       the other guy's king, it's an illegal position and
       we need to take the move back) */

    positions->ply++;
    positions->hply++;

    positions->side_to_move = static_cast<Color>(!(positions->side_to_move));
            // ~positions->side_to_move;
            // static_cast<Color>(!(positions->side_to_move));
    zobrist->change_side(positions->hash);

    //    std::cout << "make move : " << move->show() << "  test légalité du coup joué par " << ppos->side_to_move << std::endl;

    if (in_check( side ))
    {
        unmake_move(move);
        return(false);
    }
    else
    {
        // Vérification du hash code

//        U64 hash = 0;
//        zobrist->set_key(hash, positions, board);

//        if (hash != positions->hash)
//        {
//            Move a = *move;
//            string s = a.show();

//            std::cout << "erreur hash : " << s  << std::endl;
//            printf("cap=%d p2=%d cas=%d pro=%d \n", move->capture(), move->pawn2(), move->castle(), move->promotion());
//            exit(1);
//        }
        return(true);
    }

}

//=========================================================
//! \brief  Défait un coup sur l'échiquier
//! \param[in] pos      position dans laquelle est effectué le coup
//! \param[in] move     coup à effectuer
//---------------------------------------------------------
void Board::unmake_move(const Move* move)
{
    assert(move != nullptr);

    Square from = move->from();
    assert (from>=A1 && from<=H8);

    Square dest = move->dest();
    assert (dest>=A1 && from<=H8);

    Color       side = positions->side_to_move;

    positions->ply--;
    positions->hply--;

    positions->side_to_move = static_cast<Color>(!(positions->side_to_move));
            // ~positions->side_to_move;
            // static_cast<Color>(!(positions->side_to_move));


    // Récupération des caractéristiques
    positions->ep_square    = history[positions->hply].ep_square;
    positions->ep_took      = history[positions->hply].ep_took;
    positions->castle       = history[positions->hply].castle;
    positions->fifty        = history[positions->hply].fifty;
    positions->hash         = history[positions->hply].hash;


    if (move->castle())
    {
        switch(dest)
        {
        case G1:                        // petit roque blanc
            board[H1] = board[F1];
            board[H1]->setSquare(H1);
            board[F1] = &(vide[F1]);
            break;
        case C1:                        // grand roque blanc
            board[A1] = board[D1];
            board[A1]->setSquare(A1);
            board[D1] = &(vide[D1]);
            break;

        case C8:                        // petit roque noir
            board[A8] = board[D8];
            board[A8]->setSquare(A8);
            board[D8] = &(vide[D8]);
            break;

        case G8:                        // grand roque noir
            board[H8] = board[F8];
            board[H8]->setSquare(H8);
            board[F8] = &(vide[F8]);
            break;
        default:
            break;
        }

        board[from] = board[dest];
        board[from]->setSquare(from);
        board[dest] = &(vide[dest]);
    }

    // Case de prise en passant (on a joué e2-e4)
    else if (move->pawn2())
    {
        board[from] = board[dest];
        board[from]->setSquare(from);
        board[dest] = &(vide[dest]);
    }

    // Capture
    else if (move->capture())
    {
//                printf("side=%d undomove capture     : f=%d d=%d t=%d \n", positions->side_to_move, from, dest, board[dest]->type());

        if (move->pep())
        {
            Color     xside = static_cast<Color>(!side);

            board[from] = board[dest];                  // pion prenant
            board[from]->setSquare(from);
            board[dest] = &(vide[dest]);

            board[positions->ep_took] = history[positions->hply].captured;
            assert(board[positions->ep_took]);
            board[positions->ep_took]->setDead(false);
        }
        else
        {
            PieceType type = board[dest]->type();   // type de la pièce prise

            board[from] = board[dest];              // from = e7 ; dest = h7
            board[from]->setSquare(from);

            board[dest] = history[positions->hply].captured;
            assert(board[dest]);
            board[dest]->setDead(false);

            if (move->promotion())
                board[from]->setType(PAWN);
        }
    }

    // Déplacement
    else
    {
        PieceType type = board[dest]->type();

        board[from] = board[dest];
        board[from]->setSquare(from);
        board[dest] = &(vide[dest]);

        if (move->promotion())
            board[from]->setType(PAWN);
    }
}

void Board::verif(const std::string& msg, Position* pos)
{


    bool ko = false;
    for (int index = first_move[positions->ply]; index < first_move[positions->ply + 1]; ++index)
    {
        if (moves[index].type()<EMPTY || moves[index].type()>KING)
        {
            U32 code = moves[index].code();
            std::string binary = std::bitset<32>(code).to_string(); //to binary
            std::cout<<"code   " << binary<<"\n";

            printf("(%s) %d piece inconnue \n", msg.c_str(), index);
            printf("depth=%d side=%d index=%d (%d-%d) type=%d from=%d dest=%d \n",
                   pos->ply, pos->side_to_move, index, first_move[positions->ply], first_move[positions->ply+1],
                    moves[index].type(), moves[index].from(), moves[index].dest());
            ko = true;
        }
        if (moves[index].type()==EMPTY)
        {
            U32 code = moves[index].code();
            std::string binary = std::bitset<32>(code).to_string(); //to binary
            std::cout<<"code   " << binary<<"\n";

            printf("(%s) %d piece vide \n", msg.c_str(), index);
            printf("depth=%d side=%d index=%d (%d-%d) type=%d from=%d dest=%d \n",
                   pos->ply, pos->side_to_move, index, first_move[positions->ply], first_move[positions->ply+1], moves[index].type(), moves[index].from(), moves[index].dest());
            ko = true;
        }
        if (moves[index].from()<A1 || moves[index].from()>H8)
        {
            U32 code = moves[index].code();
            std::string binary = std::bitset<32>(code).to_string(); //to binary
            std::cout<<"code   " << binary<<"\n";

            printf("(%s) %d piece from \n", msg.c_str(), moves[index].from());
            printf("depth=%d side=%d index=%d (%d - %d) type=%d from=%d dest=%d \n",
                   pos->ply, pos->side_to_move, index, first_move[positions->ply], first_move[positions->ply+1],
                    moves[index].type(), moves[index].from(), moves[index].dest());
            ko = true;
        }
        if (moves[index].dest()<A1 || moves[index].dest()>H8)
        {
            U32 code = moves[index].code();
            std::string binary = std::bitset<32>(code).to_string(); //to binary
            std::cout<<"code   " << binary<<"\n";

            printf("(%s) %d piece dest \n", msg.c_str(), moves[index].dest());
            printf("depth=%d side=%d index=%d (%d-%d) type=%d from=%d dest=%d \n", pos->ply, pos->side_to_move,
                   index, first_move[positions->ply], first_move[positions->ply+1], moves[index].type(), moves[index].from(), moves[index].dest());
            ko = true;
        }
    }
    if (ko)
    {
        display_ascii();
//        printf("pv");
//        for (int j = 0; j < pv_length[0]; ++j)
//            printf(" %s", pv[0][j].c_str());
//        printf("\n");
//        fflush(stdout);

        exit(1);

    }

}
// https://www.chessprogramming.org/Null_Move_Pruning
// https://mediocrechess.blogspot.com/2007/01/guide-null-moves.html

void Board::make_null_move()
{
    // un Null Move est un coup où on passe son tour.
    // la position ne change pas, excepté le camp qui change.

//    assert(CheckBoard(pos));
//    assert(!SqAttacked(pos->KingSq[pos->side], pos->side^1,pos));

    // Sauvegarde des caractéristiques de la position
    history[positions->hply].ep_square  = positions->ep_square;
    history[positions->hply].ep_took    = positions->ep_took;
    history[positions->hply].castle     = positions->castle;
    history[positions->hply].fifty      = positions->fifty;
    history[positions->hply].hash       = positions->hash;
    // Sauvegarde effectuée

    // hash enpassant if available (remove enpassant square from hash key )
    if (positions->ep_square != OFFBOARD)
        zobrist->update_en_passant(positions->hash, positions->ep_square);

    // reset enpassant square
    positions->ep_square = OFFBOARD;

    // gnii, ça m'a pris du temps cette petite ligne !!!
    first_move[positions->ply + 1] = first_move[positions->ply];

    positions->ply++;
    positions->hply++;

    positions->side_to_move = static_cast<Color>(!(positions->side_to_move));
    zobrist->change_side(positions->hash);


//    ASSERT(CheckBoard(pos));
//    ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXGAMEMOVES);
//    ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);

    return;
}

void Board::take_null_move()
{
//    ASSERT(CheckBoard(pos));

    positions->ply--;
    positions->hply--;
    positions->side_to_move = static_cast<Color>(!(positions->side_to_move));

    // Récupération des caractéristiques
    positions->ep_square    = history[positions->hply].ep_square;
    positions->ep_took      = history[positions->hply].ep_took;
    positions->castle       = history[positions->hply].castle;
    positions->fifty        = history[positions->hply].fifty;
    positions->hash         = history[positions->hply].hash;


//    ASSERT(CheckBoard(pos));
//    ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXGAMEMOVES);
//    ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);
}













