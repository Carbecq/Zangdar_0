#include "evaluation.h"
#include "Board.h"
#include "defines.h"
#include "MoveGen.h"
#include <iostream>

// isolated pawn masks [square]
extern Bitboard isolated_bitmasks[8];

// passed pawn masks [side][square]
extern Bitboard passed_pawn_masks[2][64];
extern Bitboard outpost_masks[2][64];
extern Bitboard rear_span_masks[2][64];
extern Bitboard backwards_masks[2][64];
extern Bitboard outer_kingring[64];

//==========================================
//! \brief  Evaluation de la position
//------------------------------------------
[[nodiscard]] int Board::evaluate()
{
    int mg[2];
    int eg[2];
    int gamePhase = 0;

    // nullité
    // voir le code de Sjeng (aussi Fruit-Mora), qui comporte un test s'il reste des pions
    //  if (MaterialDraw() == true)
    //    return 0;

    evaluate_0<WHITE>(mg[WHITE], eg[WHITE], gamePhase);
    evaluate_0<BLACK>(mg[BLACK], eg[BLACK], gamePhase);

    //    printf("mgw=%d mgb=%d \n", mg[WHITE], mg[BLACK]);
    //    printf("egw=%d egb=%d \n", eg[WHITE], eg[BLACK]);

    int mgScore = mg[WHITE] - mg[BLACK];
    int egScore = eg[WHITE] - eg[BLACK];
    int mgPhase = gamePhase;

    if (mgPhase > 24)
        mgPhase = 24; // in case of early promotion
    int egPhase = 24 - mgPhase;

    I32 score = (mgScore * mgPhase + egScore * egPhase) / 24;

    //    printf("side=%d : mgp = %d ; egp = %d : s = %d \n", side_to_move, mgPhase, egPhase, score);

    // return score relative to the side to move
    if (side_to_move == WHITE)
        return score;
    else
        return -score;
}

// Code de Vice (chap 82)
//  >> vient de Sjeng 11.2 (draw.c) et neval.c , ligne 588
//  >> qui vient de Faile
//  8/6R1/2k5/6P1/8/8/4nP2/6K1 w - - 1 41   : position vue dns la vidéo de Vice,
//  à qui sert-elle ?

template<Color Us>
constexpr void Board::evaluate_0(int &mg, int &eg, int &gamePhase)
{
    int sq, sqpos;
    //   int nbr;

    // nullité
    // voir le code de Sjeng, qui comporte un test s'il reste des pions
    //  if (MaterialDraw() == true)
    //    return 0;

    mg = 0;
    eg = 0;
#ifdef NEW_EVAL
    constexpr Color Them = ~Us;
    const Bitboard occupiedBB = occupied(); // toutes les pièces (Blanches + Noires)
    const Bitboard emptyBB = ~occupiedBB;

    Bitboard enemyBB = colorPiecesBB[Them];
    Bitboard valides = emptyBB | enemyBB;

    Bitboard UsPawnsBB = pieces_cp<Us, PieceType::Pawn>();
    Bitboard OtherPawnsBB = pieces_cp<Them, PieceType::Pawn>();
#endif
    {
        constexpr PieceType pt = PieceType::Pawn;
        Bitboard bb = pieces_cp<Us, PieceType::Pawn>();
        while (bb) {
            // score matériel
            mg += mg_value[pt];
            eg += eg_value[pt];

            // case où est la pièce
            sq = next_square(bb);

            // score positionnel
            // il faut intervertir les cases pour les Noirs
            sqpos = Square::flip(Us, sq);
#ifdef DEBUG_EVAL
            //    printf("le pion %s (%d) sur la case %s a une valeur MG de %d \n", side_name[Us].c_str(), Us, square_name[sq].c_str(), mg_pawn_table[sqpos]);
#endif

            mg += mg_pawn_table[sqpos];
            eg += eg_pawn_table[sqpos];

#ifdef NEW_EVAL
            // Pions doublés
            // des pions doublés désignent deux pions de la même couleur sur une même colonne.
            // note : FileMask ne contient PAS le pion à "sq"
            if (Bcount(UsPawnsBB & FileMask64[sq]) > 0) {
#ifdef DEBUG_EVAL
                printf("le pion %s (%d) sur la case %s est doublé \n",
                       side_name[Us].c_str(),
                       Us,
                       square_name[sq].c_str());
#endif
                mg += mg_doubled_pawn;
                eg += eg_doubled_pawn;
            }

            // Pion isolé
            //  Un pion isolé est un pion qui n'a plus de pion de son camp sur les colonnes adjacentes.
            //  Un pion isolé peut être redoutable en milieu de partie1.
            //  C'est souvent une faiblesse en finale, car il est difficile à défendre.
            if ((UsPawnsBB & isolated_bitmasks[Square::file(sq)]) == 0) {
#ifdef DEBUG_EVAL
                printf("le pion %s sur la case %s est isolé \n",
                       side_name[Us].c_str(),
                       square_name[sq].c_str());
#endif
                mg += mg_isolated_pawn;
                eg += eg_isolated_pawn;
            }

            // Pion passé
            //  un pion passé est un pion qui n'est pas gêné dans son avance vers la 8e rangée par un pion adverse,
            //  c'est-à-dire qu'il n'y a pas de pion adverse devant lui, ni sur la même colonne, ni sur une colonne adjacente
            if ((passed_pawn_masks[Us][sq] & OtherPawnsBB) == 0) {
#ifdef DEBUG_EVAL
                printf("le pion %s sur la case %s est passé \n",
                       side_name[Us].c_str(),
                       square_name[sq].c_str());
                printf("sq=%d rank=%d bonus=%d \n",
                       sqpos,
                       sqpos / 8,
                       mg_passed_pawn[Square::rank(sqpos)]);
#endif
                mg += mg_passed_pawn[Square::rank(sqpos)];
                eg += eg_passed_pawn[Square::rank(sqpos)];
            }

            // Pion arriéré
            //  un pion arriéré est un pion qui est moins avancé que ceux des colonnes adjacentes
            //  et ne peut plus bénéficier de leur protection1.
            if ((backwards_masks[Us][sq] & UsPawnsBB) == 0) {
#ifdef DEBUG_EVAL
                printf("le pion %s sur la case %s est arriéré \n",
                       side_name[Us].c_str(),
                       square_name[sq].c_str());
                printf("sq=%d bonus=%d \n", sqpos, mg_backward_pawn);
#endif
                mg += mg_backward_pawn;
                eg += eg_backward_pawn;
            }

            // Pions pendants
            //  On appelle pions pendants deux pions centraux situés côte à côte sur la même rangée,
            //  sans pion de leur camp sur les colonnes adjacentes et sans pion adverse sur leurs colonnes.
#endif
        }
    }

    {
        constexpr PieceType pt = PieceType::Knight;
        Bitboard bb = pieces_cp<Us, PieceType::Knight>();
        while (bb) {
            gamePhase += 1;

            // score matériel
            mg += mg_value[pt];
            eg += eg_value[pt];

            // case où est la pièce
            sq = next_square(bb);

            // score positionnel
            // il faut intervertir les cases
            // car les tables sont données à l'envers
            sqpos = Square::flip(Us, sq);

            mg += mg_knight_table[sqpos];
            eg += eg_knight_table[sqpos];
#ifdef NEW_EVAL
            // mobilité
            int mobility = Bcount(MoveGen::knight_moves(sq) & valides);

            mg += (mobility - knight_unit) * mg_knight_mobility;
            eg += (mobility - knight_unit) * eg_knight_mobility;
#endif
        }
    }

    {
        constexpr PieceType pt = PieceType::Bishop;
        Bitboard bb = pieces_cp<Us, PieceType::Bishop>();

#ifdef NEW_EVAL
        if (Bcount(bb) >= 0) {
            mg += mg_bishop_pair;
            eg += eg_bishop_pair;
        }
#endif
        while (bb) {
            gamePhase += 1;

            // score matériel
            mg += mg_value[pt];
            eg += eg_value[pt];

            // case où est la pièce
            sq = next_square(bb);

            // score positionnel
            // il faut intervertir les cases
            sqpos = Square::flip(Us, sq);

            mg += mg_bishop_table[sqpos];
            eg += eg_bishop_table[sqpos];

#ifdef NEW_EVAL
            // mobilité
            int mobility = Bcount(MoveGen::bishop_moves(sq, occupiedBB) & valides);

            mg += (mobility - bishop_unit) * mg_bishop_mobility;
            eg += (mobility - bishop_unit) * eg_bishop_mobility;
#endif
        }
    }

    {
        constexpr PieceType pt = PieceType::Rook;
        Bitboard bb = pieces_cp<Us, PieceType::Rook>();
        while (bb)
        {
            gamePhase += 2;

            // score matériel
            mg += mg_value[pt];
            eg += eg_value[pt];

            // case où est la pièce
            sq = next_square(bb);

            // score positionnel
            // il faut intervertir les cases
            // car les tables sont données à l'envers
            sqpos = Square::flip(Us, sq);

            mg += mg_rook_table[sqpos];
            eg += eg_rook_table[sqpos];

#ifdef NEW_EVAL
            // mobilité
            int mobility = Bcount(MoveGen::rook_moves(sq, occupiedBB) & valides);

            mg += (mobility - rook_unit) * mg_rook_mobility;
            eg += (mobility - rook_unit) * eg_rook_mobility;

            // Files
            if (UseOpenFile) {
                mg -= mg_open_file / 2;
                eg -= eg_open_file / 2;

                // semi-open file
                if ((UsPawnsBB & FileMask64[sq]) == 0) // pas de pion ami
                {
#ifdef DEBUG_EVAL
                    printf("la tour %s sur la case %s est sur une colonne semi-ouverte \n",
                           side_name[Us].c_str(),
                           square_name[sq].c_str());
#endif
                    mg += mg_semiopen_file;
                    eg += eg_semiopen_file;

                    // open file
                    if ((OtherPawnsBB & FileMask64[sq]) == 0) // pas de pion ennemi
                    {
#ifdef DEBUG_EVAL
                        printf("la tour %s sur la case %s est sur une colonne ouverte \n",
                               side_name[Us].c_str(),
                               square_name[sq].c_str());
#endif
                        mg += mg_open_file - mg_semiopen_file;
                        eg += eg_open_file - eg_semiopen_file;
                    }

                    // attaque du roi ennemi
                    int rook_file = Square::file(sq);
                    int king = x_king[Them];
                    int king_file = Square::file(king);
                    int delta = abs(rook_file - king_file); // file distance

                    if (delta <= 1) {
                        mg += mg_king_semiopen_file;
                        if (delta == 0)
                            mg += mg_king_open_file - mg_king_semiopen_file;
                    }
                }
            }

#endif
        }
    }

    {
        constexpr PieceType pt = PieceType::Queen;
        Bitboard bb = pieces_cp<Us, PieceType::Queen>();
        while (bb) {
            gamePhase += 4;

            // score matériel
            mg += mg_value[pt];
            eg += eg_value[pt];

            // case où est la pièce
            sq = next_square(bb);

            // score positionnel
            // il faut intervertir les cases
            sqpos = Square::flip(Us, sq);

            mg += mg_queen_table[sqpos];
            eg += eg_queen_table[sqpos];

#ifdef NEW_EVAL
            // mobilité
            int mobility = Bcount(MoveGen::queen_moves(sq, occupiedBB) & valides);

            mg += (mobility - queen_unit) * mg_queen_mobility;
            eg += (mobility - queen_unit) * eg_queen_mobility;
#endif
        }
    }

    {
        constexpr PieceType pt = PieceType::King;
        Bitboard bb = pieces_cp<Us, PieceType::King>();
        while (bb) {
            // score matériel
            mg += mg_value[pt];
            eg += eg_value[pt];

            // case où est la pièce
            sq = next_square(bb);

            // score positionnel
            // il faut intervertir les cases
            sqpos = Square::flip(Us, sq);

            mg += mg_king_table[sqpos];
            eg += eg_king_table[sqpos];
        }
    }
}

template void Board::evaluate_0<WHITE>(int &mg, int &eg, int &gamePhase);
template void Board::evaluate_0<BLACK>(int &mg, int &eg, int &gamePhase);
