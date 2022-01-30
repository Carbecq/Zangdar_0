#include <cassert>
#include <sstream>

#include "defines.h"
#include "Board.h"
#include "Zobrist.h"

//-----------------------------------------------------
//! \brief Initialisation depuis une position FEN
//-----------------------------------------------------
void Board::init_fen(const std::string& fen, bool logTactics)
{
#ifdef DEBUG_CLASS
    std::cout << "Board::init_fen" << std::endl;
#endif

    //    rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    //                                                1 2    3 4 5
    //
    //      1) w       white to move
    //      2) KQkq    roques possibles, ou '-'
    //      3) -       case en passant
    //      4) 0       demi-coups pour la règle des 50 coups
    //      5) 1       nombre de coups de la partie

    // EPD notation : extension de FEN

    //    rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - <opération>;
    //                                                1 2    3
    //
    //      1) w       white to move
    //      2) KQkq    roques possibles, ou '-'
    //      3) -       case en passant
    //      4) ...     opération, par exemple : bm Re6; id WAC10";

    assert(fen.length() > 0);

    // est-ce une notation FEN ou EPD ?
    bool epd = false;
    std::size_t found = fen.find(';');
    if (found != std::string::npos)
        epd = true;

    //-------------------------------------

    // Reset the board
    reset();

    // Initialise position
    positions->init(WHITE);

    //-------------------------------------

    std::istringstream iss(fen);

    // Parse board positions
    Square s = A8;
    std::string position;
    iss >> position;
    for (auto it = position.begin(); it != position.end(); ++it)
    {
        char sq = *it;
        if (sq == '/')
        {
            s = Square(s - 24); // New rank
        }
        else if ('1' <= sq && sq <= '8')
        {   // Empty Squares
            s = Square(s + sq - '1' + 1); // Next Square
        }
        else
        {   // Non EMPTY Square
            PieceType t = EMPTY;
            Color     c = WHITE;

            switch (sq)
            {
            case 'p':
                t = PAWN;
                c = BLACK;
                break;
            case 'n':
                t = KNIGHT;
                c = BLACK;
                break;
            case 'b':
                t = BISHOP;
                c = BLACK;
                break;
            case 'r':
                t = ROOK;
                c = BLACK;
                break;
            case 'q':
                t = QUEEN;
                c = BLACK;
                break;
            case 'k':
                t = KING;
                c = BLACK;
                break;
            case 'P':
                t = PAWN;
                c = WHITE;
                break;
            case 'N':
                t = KNIGHT;
                c = WHITE;
                break;
            case 'B':
                t = BISHOP;
                c = WHITE;
                break;
            case 'R':
                t = ROOK;
                c = WHITE;
                break;
            case 'Q':
                t = QUEEN;
                c = WHITE;
                break;
            case 'K':
                t = KING;
                c = WHITE;
                break;
            default:
                assert(false);
            }
            add_piece(t, c, s);

            s = Square(s + 1); // Next Square
        }
    }
    assert(s == Square(H1 + 1));

    //----------------------------------------------------------
    // Set the side to move

    char side;
    iss >> side;
    assert(side == 'w' || side == 'b');

    switch(side)
    {
    case 'w':
        positions->side_to_move = WHITE;
        break;
    case 'b':
        positions->side_to_move = BLACK;
        break;
    default:
        break;
    }

    //----------------------------------------------------------
    // droit au roque

    std::string castling;
    iss >> castling;

    for (auto it = castling.begin(); it != castling.end(); ++it)
    {
        switch(*it)
        {
        case 'K':
            positions->castle |= CASTLE_WK;
            break;
        case 'Q':
            positions->castle |= CASTLE_WQ;
            break;
        case 'k':
            positions->castle |= CASTLE_BK;
            break;
        case 'q':
            positions->castle |= CASTLE_BQ;
            break;
        case '-':
            break;
        }
    }
    assert(positions->castle >=0 && positions->castle <= 15);

    //----------------------------------------------------------
    // prise en passant

    std::string ep;
    iss >> ep;
    if (ep != "-")
    {
        char file = ep.at(0);
        char rank = ep.at(1);
        s = Square((rank - '1') * 16 + file - 'a');
        assert(!is_out(s));
        positions->ep_square = s;

        if (positions->side_to_move == WHITE)
            positions->ep_took = static_cast<Square>(s - 16);
        else
            positions->ep_took = static_cast<Square>(s + 16);
    }

    //-----------------------------------------
    // la lecture dépend si on a une notation EPD ou FEN

    if (epd == true)
    {
        // iss : bm Rd6; id "WAC12";

        // best move
        std::string op, auxi;
        size_t p;

        iss >> op;
        if (op == "bm")
        {
            // on peut avoir plusieurs meilleurs coups !
            positions->best.clear();
            while(true)
            {
                iss >> auxi;
                p = auxi.find(';');     // indique la fin du champ "bm"
                if (p == std::string::npos)  // pas trouvé
                {
                    positions->best.push_back(auxi);
                }
                else
                {
                    std::string bm = auxi.substr(0, p);
                    positions->best.push_back(bm);
                    break;
                }
            }
        }

        // identifiant
        iss >> op;
        if (op == "id")
        {
            iss >> auxi;
            p = auxi.find(';');
            std::string ident = auxi.substr(0, p);

            if (logTactics)
                std::cout << ident << " : ";
        }
    }
    else
    {
        int halfmove = 0;
        iss >> halfmove;
        //    current_position().set_halfmove(halfmove);

        int fullmove = 1;
        iss >> fullmove;
        //  int ply = 2 * (fullmove - 1);
        //    if (current_position().side() == BLACK) {
        //        ++ply;
        //    }
        //    tree.set_ply(ply);

    }

    // Met à jour l'échiquier
    update();

    //    display_ascii();

    // Clef Zobrist de la position
    positions->hash = 0;
    zobrist->set_key(positions->hash, positions, board);

  //  display_ascii();

    init_MVVLVA();


}

//=================================================================
//! \brief  Ajout d'une pièce à l'échiquier
//! \param[in] t    type de la pièce
//! \param[in] c    couleur de la pièce
//! \param[in] s    case où se trouve la pièce
//-----------------------------------------------------------------
void Board::add_piece(PieceType t, Color c, Square s)
{
    pieces[c].push_back( Piece(t, c, s) );

//        if (c == WHITE)
//            printf("piece W : t=%d s=%d \n", t, s);
//        else
//            printf("piece B : t=%d s=%d \n", t, s);



    // ATTENTION : on ne peut pas mettre ici : board[s] = &piece[c][n]
    //  car l'adresse dans le vector pieces changera au fur et à mesure des push-back

    // ou alors, il faudrait utiliser l'index de la pièce




    //    pieces.inc_nb_pieces(c, t);

    // Update Zobrist hash
    //->    Position& pos = current_position();
    //->    zobrist.update_piece(pos.hash(), c, t, s);

    // Hack: for the material hash, the position is irrelevant but each piece
    // needs a unique hash so we are using Square(i) in place of the position.
    // Remove the previous total
    //->    zobrist.update_piece(pos.material_hash(), c, t, Square(i));
    // Add the new total
    //->    zobrist.update_piece(pos.material_hash(), c, t, Square(i + 1));
}

void Board::update()
{
    Piece  p;
    Square s;

    for (auto c : COLORS)
    {
        // Déplacement du roi comme première pièce
        // Utile lorsqu'on cherche si le camp est en échec
        for (int n=0; n<pieces[c].size(); n++)
        {
            if (pieces[c][n].type() == KING)
            {
                p = pieces[c][0];
                pieces[c][0] = pieces[c][n];
                pieces[c][n] = p;
                break;
            }
        }

        // Affectation des pièces dans l'échiquier
        for (int n=0; n<pieces[c].size(); n++)
        {
            s = pieces[c][n].square();
            board[s] = &(pieces[c][n]);
        }
    }
}

//=========================================================================
//  Lecture d'une position fen, mais on va inverser l'échiquier
//  Ceci permet de tester l'évaluation, qui doit être symétrique.
//-------------------------------------------------------------------------
void Board::mirror_fen(const std::string& fen)
{
    //    rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    //                                                1 2    3 4 5
    //
    //      1) w       white to move
    //      2) KQkq    roques possibles, ou '-'
    //      3) -       case en passant
    //      4) 0       demi-coups pour la règle des 50 coups
    //      5) 1       nombre de coups de la partie

    // EPD notatin : extension de FEN

    //    rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - <opération>;
    //                                                1 2    3
    //
    //      1) w       white to move
    //      2) KQkq    roques possibles, ou '-'
    //      3) -       case en passant
    //      4) ...     opération, par exemple : bm Re6; id WAC10";

    assert(fen.length() > 0);

    // est-ce une notation FEN ou EPD ?
    bool epd = false;
    std::size_t found = fen.find(';');
    if (found != std::string::npos)
        epd = true;

    //-------------------------------------

    // Reset the board
    reset();

    // Initialise position
    positions->init(WHITE);

    //-------------------------------------

    std::istringstream iss(fen);

    // Parse board positions
    Square s = A8;
    std::string position;
    iss >> position;
    for (auto it = position.begin(); it != position.end(); ++it)
    {
        char sq = *it;
        if (sq == '/')
        {
            s = Square(s - 24); // New rank
        }
        else if ('1' <= sq && sq <= '8')
        {   // Empty Squares
            s = Square(s + sq - '1' + 1); // Next Square
        }
        else
        {   // Non EMPTY Square
            PieceType t = EMPTY;
            Color     c = WHITE;

            switch (sq)
            {
            case 'p':
                t = PAWN;
                c = BLACK;
                break;
            case 'n':
                t = KNIGHT;
                c = BLACK;
                break;
            case 'b':
                t = BISHOP;
                c = BLACK;
                break;
            case 'r':
                t = ROOK;
                c = BLACK;
                break;
            case 'q':
                t = QUEEN;
                c = BLACK;
                break;
            case 'k':
                t = KING;
                c = BLACK;
                break;
            case 'P':
                t = PAWN;
                c = WHITE;
                break;
            case 'N':
                t = KNIGHT;
                c = WHITE;
                break;
            case 'B':
                t = BISHOP;
                c = WHITE;
                break;
            case 'R':
                t = ROOK;
                c = WHITE;
                break;
            case 'Q':
                t = QUEEN;
                c = WHITE;
                break;
            case 'K':
                t = KING;
                c = WHITE;
                break;
            default:
                assert(false);
            }

            add_piece(t, ~c, MIRROR_SQUARES[s]);

            s = Square(s + 1); // Next Square
        }
    }
    assert(s == Square(H1 + 1));

    //----------------------------------------------------------
    // Set the side to move

    char side;
    iss >> side;
    assert(side == 'w' || side == 'b');

    switch(side)
    {
    case 'w':
        positions->side_to_move = BLACK;
        break;
    case 'b':
        positions->side_to_move = WHITE;
        break;
    default:
        break;
    }

    //----------------------------------------------------------
    // droit au roque

    std::string castling;
    iss >> castling;

    for (auto it = castling.begin(); it != castling.end(); ++it)
    {
        switch(*it)
        {
        case 'K':
            positions->castle |= CASTLE_BK;
            break;
        case 'Q':
            positions->castle |= CASTLE_BQ;
            break;
        case 'k':
            positions->castle |= CASTLE_WK;
            break;
        case 'q':
            positions->castle |= CASTLE_WQ;
            break;
        case '-':
            break;
        }
    }
    assert(positions->castle >=0 && positions->castle <= 15);

    //----------------------------------------------------------
    // prise en passant

    std::string ep;
    iss >> ep;
    if (ep != "-")
    {
        char file = ep.at(0);
        char rank = ep.at(1);
        s = Square((rank - '1') * 16 + file - 'a');

        Square ms = MIRROR_SQUARES[s];
        assert(!is_out(ms));
        positions->ep_square = ms;

        if (positions->side_to_move == WHITE)
            positions->ep_took = static_cast<Square>(s + 16);
        else
            positions->ep_took = static_cast<Square>(s - 16);
    }

    //-----------------------------------------
    // la lecture dépend si on a une notation EPD ou FEN

    if (epd == true)
    {
        // iss : bm Rd6; id "WAC12";

        // best move
        std::string op, auxi;
        size_t p;

        iss >> op;
        if (op == "bm")
        {
            // on peut avoir plusieurs meilleurs coups !
            positions->best.clear();
            while(true)
            {
                iss >> auxi;
                p = auxi.find(';');     // indique la fin du champ "bm"
                if (p == std::string::npos)  // pas trouvé
                {
                    positions->best.push_back(auxi);
                }
                else
                {
                    std::string bm = auxi.substr(0, p);
                    positions->best.push_back(bm);
                    break;
                }
            }
        }

        // identifiant
        iss >> op;
        if (op == "id")
        {
            iss >> auxi;
            p = auxi.find(';');
            std::string ident = auxi.substr(0, p);
        }
    }
    else
    {
        int halfmove = 0;
        iss >> halfmove;
        //    current_position().set_halfmove(halfmove);

        int fullmove = 1;
        iss >> fullmove;
        //  int ply = 2 * (fullmove - 1);
        //    if (current_position().side() == BLACK) {
        //        ++ply;
        //    }
        //    tree.set_ply(ply);

    }

    // Met à jour l'échiquier
    update();

    //    display_ascii();

    // Clef Zobrist de la position
    positions->hash = 0;
    zobrist->set_key(positions->hash, positions, board);

//  display_ascii();

    init_MVVLVA();


}

