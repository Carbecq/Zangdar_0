#include <cassert>
#include <algorithm>
#include <sstream>
#include <string>
#include <iomanip>
#include "Board.h"
#include "Square.h"

//    rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
//                                                1 2    3 4 5
//
//      1) w       white to move
//      2) KQkq    roques possibles, ou '-'
//      3) -       case en passant
//      4) 0       demi-coups pour la règle des 50 coups   : halfmove_counter
//      5) 1       nombre de coups de la partie            : fullmove_counter

// The halfmove clock specifies a decimal number of half moves with respect to the 50 move draw rule.
// It is reset to zero after a capture or a pawn move and incremented otherwise.

// The number of the full moves in a game.
// It starts at 1, and is incremented after each Black's move

// EPD notation : extension de FEN

//    rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - <opération>;
//                                                1 2    3
//
//      1) w       white to move
//      2) KQkq    roques possibles, ou '-'
//      3) -       case en passant
//      4) ...     opération, par exemple : bm Re6; id WAC10";

//-----------------------------------------------------
//! \brief Initialisation depuis une position FEN
//-----------------------------------------------------
void Board::set_fen(const std::string &fen, bool logTactics) noexcept
{
    assert(fen.length() > 0);

    if (fen == "startpos")
    {
        set_fen(START_FEN, logTactics);
        return;
    }

    // est-ce une notation FEN ou EPD ?
    bool epd = false;
    int count = std::count(fen.begin(), fen.end(), ';');
    if (count > 0)
        epd = true;

    //-------------------------------------

    // Reset the board
    clear();
    avoid_moves.clear();
    best_moves.clear();

    //-------------------------------------
    std::stringstream ss{fen};
    std::string word;

    ss >> word;
    int i = 56;
    for (const auto &c : word) {
        switch (c) {
        case 'P':
            set_piece(i, Color::WHITE, PieceType::Pawn);
            i++;
            break;
        case 'p':
            set_piece(i, Color::BLACK, PieceType::Pawn);
            i++;
            break;
        case 'N':
            set_piece(i, Color::WHITE, PieceType::Knight);
            i++;
            break;
        case 'n':
            set_piece(i, Color::BLACK, PieceType::Knight);
            i++;
            break;
        case 'B':
            set_piece(i, Color::WHITE, PieceType::Bishop);
            i++;
            break;
        case 'b':
            set_piece(i, Color::BLACK, PieceType::Bishop);
            i++;
            break;
        case 'R':
            set_piece(i, Color::WHITE, PieceType::Rook);
            i++;
            break;
        case 'r':
            set_piece(i, Color::BLACK, PieceType::Rook);
            i++;
            break;
        case 'Q':
            set_piece(i, Color::WHITE, PieceType::Queen);
            i++;
            break;
        case 'q':
            set_piece(i, Color::BLACK, PieceType::Queen);
            i++;
            break;
        case 'K':
            set_piece(i, Color::WHITE, PieceType::King);
            x_king[Color::WHITE] = i;
            i++;
            break;
        case 'k':
            set_piece(i, Color::BLACK, PieceType::King);
            x_king[Color::BLACK] = i;
            i++;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
            i += c - '1' + 1;
            break;
        case '/':
            i -= 16;
            break;
        default:
            break;
        }
    }

    //----------------------------------------------------------
    // Side to move
    ss >> word;
    if (word == "w") {
        side_to_move = Color::WHITE;
    } else {
        side_to_move = Color::BLACK;
    }

    //----------------------------------------------------------
    // Droit au roque
    ss >> word;

    for (auto it = word.begin(); it != word.end(); ++it)
    {
        switch(*it)
        {
        case 'K':
            castling |= CASTLE_WK;
            break;
        case 'Q':
            castling |= CASTLE_WQ;
            break;
        case 'k':
            castling |= CASTLE_BK;
            break;
        case 'q':
            castling |= CASTLE_BQ;
            break;
        case '-':
            break;
        }
    }

    //----------------------------------------------------------
    // prise en passant

    std::string ep;
    ss >> ep;
    if (ep != "-")
    {
        char file = ep.at(0);
        char rank = ep.at(1);
        int s = (rank - '1') * 8 + file - 'a';
        assert(s>=A1 && s<=H8);
        ep_square = s;
    }

    //-----------------------------------------
    // la lecture dépend si on a une notation EPD ou FEN

    if (epd == true)
    {
        // iss : bm Rd6; id "WAC12";
        //       am Rd6; bm Rb6 Rg5+; id "WAC.274";
        //       bm Bg4 Re2; c0 "Bg4 wins, but Re2 is far better."; id "WAC.252";

        // best move
        std::string op, auxi;
        size_t p;

        for (int n=0; n<count; n++)
        {
            ss >> op;

            if (op == "bm") // Best Move
            {
                 while(true)
                {
                    ss >> auxi;
                    p = auxi.find(';');             // indique la fin du champ "bm"
                    if (p == std::string::npos)     // pas trouvé
                    {
                        best_moves.push_back(auxi);
                    }
                    else
                    {
                        std::string bm = auxi.substr(0, p);
                        best_moves.push_back(bm);
                        break;
                    }
                }
            }
            else if (op == "am")    // Avoid Move
            {
                while(true)
                {
                    ss >> auxi;
                    p = auxi.find(';');             // indique la fin du champ "am"
                    if (p == std::string::npos)     // pas trouvé
                    {
                        avoid_moves.push_back(auxi);
                    }
                    else
                    {
                        std::string am = auxi.substr(0, p);
                        avoid_moves.push_back(am);
                        break;
                    }
                }
            }
            else if (op == "id")    // identifiant
            {
                std::string total;
                while(true)
                {
                    ss >> auxi;
                    p = auxi.find(';');             // indique la fin du champ "am"
                    if (p == std::string::npos)     // pas trouvé
                    {
                        total += auxi;
                        total += " ";
                    }
                    else
                    {
                        total += auxi.substr(0, p);;
                        std::string ident = total; //.substr(0, p);
                        if (logTactics)
                            std::cout << std::setw(30) << ident << " : ";
                        break;
                    }
                }
            }
            else if (op == "c0")    // commentaire
            {
                while(true)
                {
                    ss >> auxi;
                    p = auxi.find(';');             // indique la fin du champ "am"
                    if (p == std::string::npos)     // pas trouvé
                    {
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }
    else
    {
        // Halfmove clock
        // The halfmove clock specifies a decimal number of half moves with respect to the 50 move draw rule.
        // It is reset to zero after a capture or a pawn move and incremented otherwise.
        ss >> halfmove_counter;

        // Fullmove clock
        // The number of the full moves in a game. It starts at 1, and is incremented after each Black's move.
        ss >> fullmove_counter;

        // Move count: ignore and use zero, as we count since root
        gamemove_counter = 0;       // numMoves (Ethereal)
    }

//-----------------------------------------

// Calculate hash
#ifdef HASH
    calculate_hash(hash, pawn_hash);
#endif

    //   std::cout << display() << std::endl;

    if (side_to_move == WHITE)
        assert(valid<WHITE>());
    else
        assert(valid<BLACK>());

}

//=========================================================================
//  Lecture d'une position fen, mais on va inverser l'Ã©chiquier
//  Ceci permet de tester l'Ã©valuation, qui doit Ãªtre symÃ©trique.
//-------------------------------------------------------------------------
void Board::mirror_fen(const std::string& fen, bool logTactics)
{
    assert(fen.length() > 0);

    // est-ce une notation FEN ou EPD ?
    bool epd = false;
    std::size_t found = fen.find(';');
    if (found != std::string::npos)
        epd = true;

    //-------------------------------------

    // Reset the board
    clear();
    avoid_moves.clear();
    best_moves.clear();

    //-------------------------------------

    std::stringstream ss(fen);
    std::string word;

    // Parse board positions
    ss >> word;
    int i = 56;
    for (const auto &c : word) {
        switch (c) {
        case 'P':
            set_piece(Square::flip(i), ~Color::WHITE, PieceType::Pawn);
            i++;
            break;
        case 'p':
            set_piece(Square::flip(i), ~Color::BLACK, PieceType::Pawn);
            i++;
            break;
        case 'N':
            set_piece(Square::flip(i), ~Color::WHITE, PieceType::Knight);
            i++;
            break;
        case 'n':
            set_piece(Square::flip(i), ~Color::BLACK, PieceType::Knight);
            i++;
            break;
        case 'B':
            set_piece(Square::flip(i), ~Color::WHITE, PieceType::Bishop);
            i++;
            break;
        case 'b':
            set_piece(Square::flip(i), ~Color::BLACK, PieceType::Bishop);
            i++;
            break;
        case 'R':
            set_piece(Square::flip(i), ~Color::WHITE, PieceType::Rook);
            i++;
            break;
        case 'r':
            set_piece(Square::flip(i), ~Color::BLACK, PieceType::Rook);
            i++;
            break;
        case 'Q':
            set_piece(Square::flip(i), ~Color::WHITE, PieceType::Queen);
            i++;
            break;
        case 'q':
            set_piece(Square::flip(i), ~Color::BLACK, PieceType::Queen);
            i++;
            break;
        case 'K':
            set_piece(Square::flip(i), ~Color::WHITE, PieceType::King);
            x_king[~Color::WHITE] = Square::flip(i);
            i++;
            break;
        case 'k':
            set_piece(Square::flip(i), ~Color::BLACK, PieceType::King);
            x_king[~Color::BLACK] = Square::flip(i);
            i++;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
            i += c - '1' + 1;
            break;
        case '/':
            i -= 16;
            break;
        default:
            break;
        }
    }

    //----------------------------------------------------------
    // Set the side to move
    ss >> word;
    if (word == "w") {
        side_to_move = ~Color::WHITE;
    } else {
        side_to_move = ~Color::BLACK;
    }


    //----------------------------------------------------------
    // droit au roque
    ss >> word;

    for (auto it = word.begin(); it != word.end(); ++it)
    {
        switch(*it)
        {
        case 'K':
            castling |= CASTLE_BK;
            break;
        case 'Q':
            castling |= CASTLE_BQ;
            break;
        case 'k':
            castling |= CASTLE_WK;
            break;
        case 'q':
            castling |= CASTLE_WQ;
            break;
        case '-':
            break;
        }
    }


    //----------------------------------------------------------
    // prise en passant
    std::string ep;
    ss >> ep;
    if (ep != "-")
    {
        char file = ep.at(0);
        char rank = ep.at(1);
        int s = (rank - '1') * 8 + file - 'a';
        assert(s>=A1 && s<=H8);
        ep_square = Square::flip(s);
    }

    //-----------------------------------------
    // la lecture dépend si on a une notation EPD ou FEN

    if (epd == true)
    {
        // iss : bm Rd6; id "WAC12";

        // best move
        std::string op, auxi;
        size_t p;

        ss >> op;
        if (op == "bm") // Best Move
        {
            // on peut avoir plusieurs meilleurs coups !
            best_moves.clear();
            while(true)
            {
                ss >> auxi;
                p = auxi.find(';');     // indique la fin du champ "bm"
                if (p == std::string::npos)  // pas trouvé
                {
                    best_moves.push_back(auxi);
                }
                else
                {
                    std::string bm = auxi.substr(0, p);
                    best_moves.push_back(bm);
                    break;
                }
            }
        }
        else if (op == "am")    // Avoid Move
        {
            avoid_moves.clear();
            while(true)
            {
                ss >> auxi;
                p = auxi.find(';');     // indique la fin du champ "am"
                if (p == std::string::npos)  // pas trouvé
                {
                    avoid_moves.push_back(auxi);
                }
                else
                {
                    std::string am = auxi.substr(0, p);
                    avoid_moves.push_back(am);
                    break;
                }
            }
        }

        // identifiant
        ss >> op;
        if (op == "id")
        {
            ss >> auxi;
            p = auxi.find(';');
            std::string ident = auxi.substr(0, p);

            if (logTactics)
                std::cout << ident << " : ";
        }
    }
    else
    {
        // Halfmove clock
        // The halfmove clock specifies a decimal number of half moves with respect to the 50 move draw rule.
        // It is reset to zero after a capture or a pawn move and incremented otherwise.
        ss >> halfmove_counter;

        // Fullmove clock
        // The number of the full moves in a game. It starts at 1, and is incremented after each Black's move.
        ss >> fullmove_counter;

        // Move count: ignore and use zero, as we count since root
        gamemove_counter = 0;       // numMoves (Ethereal)
    }

//-----------------------------------------

// Calculate hash
#ifdef HASH
    calculate_hash(hash, pawn_hash);
#endif

    //   std::cout << display() << std::endl;

    if (side_to_move == WHITE)
        assert(valid<WHITE>());
    else
        assert(valid<BLACK>());
}

//============================================================
//! \brief  Retourne la chaine fen correspondant
//! à la position
//------------------------------------------------------------
[[nodiscard]] std::string Board::get_fen() const noexcept
{
    std::string fen;

    for (int y = 7; y >= 0; --y)
    {
        int num_empty = 0;

        for (int x = 0; x < 8; ++x)
        {
            const auto sq = Square::square(x, y);
            const PieceType piece = piece_on(sq);
            if (piece == PieceType::NO_TYPE) {
                num_empty++;
            } else {
                // Add the number of empty squares so far
                if (num_empty > 0) {
                    fen += std::to_string(num_empty);
                }
                num_empty = 0;

                fen += piece_symbol[color_on(sq)][piece];
            }
        }

        // Add the number of empty squares when we reach the end of the rank
        if (num_empty > 0) {
            fen += std::to_string(num_empty);
        }

        if (y > 0) {
            fen += "/";
        }
    }

    //------------------------------------------------------- side

    fen += (turn() == Color::WHITE) ? " w" : " b";

    //------------------------------------------------------- castling

    std::string part;
    if (can_castle_k<WHITE>())
        part += "K";
    if (can_castle_q<WHITE>())
        part += "Q";
    if (can_castle_k<BLACK>())
        part += "k";
    if (can_castle_q<BLACK>())
        part += "q";
    if (part.empty())
        part = "-";

    fen += " ";
    fen += part;

    //------------------------------------------------------- en-passant

    fen += " ";
    if (ep_square == NO_SQUARE)
        fen += "-";
    else
        fen += square_name[ep_square];

    //------------------------------------------------------- half-move

    fen += " ";
    fen += std::to_string(get_halfmove_counter());

    //------------------------------------------------------- full-move

    fen += " ";
    fen += std::to_string(get_fullmove_counter());

    return fen;
}

//-----------------------------------------------------
//! \brief Commande UCI : position
//!         Entrée d'une position
//!         et d'une série de coups
//-----------------------------------------------------
void Board::parse_position(std::istringstream &is)
{
    std::string token;

    token.clear();
    is >> token;

    bool logTactics = false;

    if (token == "startpos")
    {
        set_fen(START_FEN, logTactics);
    }
    else
    {
        std::string fen;

        while (is >> token && token != "moves")
        {
            fen += token + " ";
        }
        set_fen(fen, logTactics);
    }

    while (is >> token)
    {
        if (token == "moves")
            continue;
        if (side_to_move == WHITE)
            apply_token<WHITE>(token);
        else
            apply_token<BLACK>(token);

        // Reset move history whenever we reset the fifty move rule. This way
        // we can track all positions that are candidates for repetitions, and
        // are still able to use a fixed size for the history array (512)
        if (halfmove_counter == 0)
            gamemove_counter = 0;

    }

}


