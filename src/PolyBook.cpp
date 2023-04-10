#include <iomanip>
#include "PolyBook.h"
#include "piece.h"
#include <string>
#include "movegen.h"
#include "ThreadPool.h"

//=================================================
//! \brief  Constructeur
//-------------------------------------------------
PolyBook::PolyBook()
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "PolyBook::constructeur  ");
    printlog(message);
#endif
    entries     = nullptr;
    nbr_entries = 0;
    path        = "./";

    if (threadPool.get_useBook())
        init("book.bin");
}

//=================================================
//! \brief  Constructeur
//-------------------------------------------------
PolyBook::PolyBook(const std::string& name)
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "PolyBook::constructeur; name : %s ", name.c_str());
    printlog(message);
#endif
    entries     = nullptr;
    nbr_entries = 0;
    path        = "./";

    if (threadPool.get_useBook())
        init(name);
}

//=================================================
//! \brief  Destructeur
//-------------------------------------------------
PolyBook::~PolyBook()
{
    clean();
}

//=======================================================
//! \brief  Calcule le hash code relatif à la position
//-------------------------------------------------------
U64 PolyBook::calculate_hash(const Board& board)
{
    U64 hash = 0;
    int offset = 0;
    int us = board.side_to_move;
    int pieceNum = 0;
    int sq;
    Bitboard pieces;

    for (PieceType type : {Pawn, Knight, Bishop, Rook, Queen, King})
    {
        for (Color color : {BLACK, WHITE})
        {
            pieces = board.colorPiecesBB[color] & board.typePiecesBB[type];

            while (pieces)
            {
                sq = next_square(pieces);
                hash ^= Random64Poly[64 * pieceNum + sq];
            }
            pieceNum++;
        }
    }

    // castling
    offset = 768;

    if(board.can_castle_k<WHITE>()) hash ^= Random64Poly[offset + 0];
    if(board.can_castle_q<WHITE>()) hash ^= Random64Poly[offset + 1];
    if(board.can_castle_k<BLACK>()) hash ^= Random64Poly[offset + 2];
    if(board.can_castle_q<BLACK>()) hash ^= Random64Poly[offset + 3];

    // enpassant
    // enable en passant flag only if there si a pawn capable to
    // capture our pawn

    int epsq = board.ep_square;
    if (epsq != NO_SQUARE)
    {
        // Bitboard des attaques de pion ADVERSE
        Bitboard bb = movegen::pawn_attacks(!us, epsq);

        if (bb & board.colorPiecesBB[board.side_to_move] & board.typePiecesBB[Pawn])
        {
            int file = Square::file(epsq);
            hash ^= Random64Poly[772 + file];
        }
    }

    // side
    if(us == WHITE) {
        hash ^= Random64Poly[780];
    }
    return hash;
}

//====================================================
//! \brief  Initialisation de la bibliothèque
//----------------------------------------------------
void PolyBook::init(const std::string &name)
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "PolyBook::init : %s ", name.c_str());
    printlog(message);
#endif

    std::size_t file_size;
    threadPool.set_useBook(false);

    std::string     str_file = path + "/" + name;
    std::ifstream   file;

    file.open(str_file, std::ifstream::in | std::ifstream::binary);
    if (!file.is_open())
    {
#ifdef DEBUG_LOG
        sprintf(message, "PolyBook::init : Can't open the book file %s ", str_file.c_str());
        printlog(message);
#endif
        return ;
    }

    // Taille du fichier
    file.seekg(0, std::ios_base::end);
    file_size = file.tellg();
    if(file_size < sizeof(PolyBookEntry))
    {
#ifdef DEBUG_LOG
        sprintf(message, "PolyBook::init : No entry found  ");
        printlog(message);
#endif
        return ;
    }
    file.seekg(0, std::ios_base::beg);

    // Allocation de la bibliothèque
    nbr_entries = file_size / sizeof(PolyBookEntry);

#ifdef DEBUG_LOG
    sprintf(message, "PolyBook::init : %u Entries Found In File", nbr_entries);
    printlog(message);
#endif
    entries = new PolyBookEntry[nbr_entries];

    file.read((char*)entries, file_size);
    if (file)
    {
#ifdef DEBUG_LOG
        sprintf(message, "PolyBook::init : Read successfull");
        printlog(message);
#endif
    }
    else
    {
#ifdef DEBUG_LOG
        sprintf(message, "PolyBook::init : Read failed : only %td could be read", file.gcount());
        printlog(message);
#endif
    }
    file.close();

    if (nbr_entries > 0)
        threadPool.set_useBook(true);

#ifdef DEBUG_LOG
    sprintf(message, "PolyBook::init : use_book = %d ", threadPool.get_useBook());
    printlog(message);
#endif
}

//==================================================
//! \brief  Nettoyage avant fermeture
//--------------------------------------------------
void PolyBook::clean()
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "PolyBook::clean ");
    printlog(message);
#endif

    delete [] entries;
    entries = nullptr;
    nbr_entries = 0;
}

//=========================================================
//! \brief  Trouve un coup possible de la bibliothèque
//!         dans la position actuelle
//---------------------------------------------------------
MOVE PolyBook::get_move(const Board& board)
{
#ifdef DEBUG_LOG
    char message[100];
    sprintf(message, "PolyBook::get_move ");
    printlog(message);
#endif

    PolyBookEntry *entry;
    U16 move;
    MOVE bookMoves[MAX_BOOK_MOVES];
    MOVE tempMove = 0;
    int count = 0;

    U64 hash = calculate_hash(board);

    for(entry = entries; entry < entries + nbr_entries; entry++)
    {
        if(hash == endian_swap_u64(entry->hash))
        {
            move = endian_swap_u16(entry->move);
            tempMove = poly_to_move(move, board);
            if(tempMove != 0)
            {
                bookMoves[count++] = tempMove;
                if(count >= MAX_BOOK_MOVES)
                    break;
            }
        }
    }

    if(count != 0)
    {
        int randMove = rand() % count;
        return bookMoves[randMove];
    }
    else
    {
        return 0;
    }
}

//===============================================================
//! \brief  Conversion d'un coup au format "Polyglot"
//!         dans mon format
//---------------------------------------------------------------
MOVE PolyBook::poly_to_move(U16 polyMove, const Board& board)
{
    int ff = (polyMove >> 6) & 7;   // from_file
    int fr = (polyMove >> 9) & 7;   // from rank
    int tf = (polyMove >> 0) & 7;   // dest file
    int tr = (polyMove >> 3) & 7;   // dest rank
    int pp = (polyMove >> 12) & 7;  // promotion piece

    int from = Square::square(ff, fr);
    int dest = Square::square(tf, tr);
    PieceType piece = board.cpiece[from];
    PieceType capt  = board.cpiece[dest];
    PieceType promo = static_cast<PieceType>(pp);

    return Move::CODE(Normal, from, dest, piece, capt, promo);
}


void test_poly()
{
    Board board;
    PolyBook book;
    std::string FEN;
    U64 key, key2;


    //    Test data
    //    Here are some test keys. They were computed using Toga II and checked using the algorithm described above.

    //    starting position
    FEN="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    board.set_fen(FEN, false);

    key=0x463b96181691fc9c;
    key2 = book.calculate_hash(board);
    if (key != key2)
        printf("erreur 1\n");


    //    position after e2e4
    FEN="rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    board.set_fen(FEN, false);
    key=0x823c9b50fd114196;
    key2 = book.calculate_hash(board);
    if (key != key2)
        printf("erreur 2\n");

    //    position after e2e4 d75
    FEN="rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2";
    board.set_fen(FEN, false);
    key=0x0756b94461c50fb0;
    key2 = book.calculate_hash(board);
    if (key != key2)
        printf("erreur 3\n");

    //    position after e2e4 d7d5 e4e5
    FEN="rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2";
    board.set_fen(FEN, false);
    key=0x662fafb965db29d4;
    key2 = book.calculate_hash(board);
    if (key != key2)
        printf("erreur 4\n");

    //    position after e2e4 d7d5 e4e5 f7f5
    FEN="rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3";
    board.set_fen(FEN, false);
    key=0x22a48b5a8e47ff78;
    key2 = book.calculate_hash(board);
    if (key != key2)
        printf("erreur 5\n");

    //    position after e2e4 d7d5 e4e5 f7f5 e1e2
    FEN="rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR b kq - 0 3";
    board.set_fen(FEN, false);
    key=0x652a607ca3f242c1;
    key2 = book.calculate_hash(board);
    if (key != key2)
        printf("erreur 6\n");

    //    position after e2e4 d7d5 e4e5 f7f5 e1e2 e8f7
    FEN="rnbq1bnr/ppp1pkpp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR w - - 0 4";
    board.set_fen(FEN, false);
    key=0x00fdd303c946bdd9;
    key2 = book.calculate_hash(board);
    if (key != key2)
        printf("erreur 7\n");

    //    position after a2a4 b7b5 h2h4 b5b4 c2c4
    FEN="rnbqkbnr/p1pppppp/8/8/PpP4P/8/1P1PPPP1/RNBQKBNR b KQkq c3 0 3";
    board.set_fen(FEN, false);
    key=0x3c8123ea7b067637;
    key2 = book.calculate_hash(board);
    if (key != key2)
        printf("erreur 8\n");

    //    position after a2a4 b7b5 h2h4 b5b4 c2c4 b4c3 a1a3
    FEN="rnbqkbnr/p1pppppp/8/8/P6P/R1p5/1P1PPPP1/1NBQKBNR b Kkq - 0 4";
    board.set_fen(FEN, false);
    key=0x5c3f9b829b279560;
    key2 = book.calculate_hash(board);
    if (key != key2)
        printf("erreur 9\n");
}
