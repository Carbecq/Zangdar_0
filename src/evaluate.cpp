#include <iostream>
#include "Board.h"
#include "defines.h"
#include "evaluate.h"
#include "TranspositionTable.h"

//==========================================
//! \brief  Evaluation de la position
//------------------------------------------
template<bool Mode>
[[nodiscard]] int Board::evaluate()
{
    int gamePhase = 0;
    int mgScore;
    int egScore;

    // L'évaluation est-elle en cache ?
#ifdef USE_CACHE
    int hashed;
    if (Transtable.probe_evaluation(hash, side_to_move, hashed) == true)
        return hashed; // + Tempo;
#endif

    // nullité
    // voir le code de Sjeng (aussi Fruit-Mora), qui comporte un test s'il reste des pions
    //  if (MaterialDraw() == true)
    //    return 0;

    if (Mode)
    {
        Score score = slow_evaluate(gamePhase);

        mgScore = MgScore(score);
        egScore = EgScore(score);
    }
    else
    {
        Score scores[2];

        fast_evaluate<WHITE>(scores[WHITE], gamePhase);
        fast_evaluate<BLACK>(scores[BLACK], gamePhase);

        mgScore = MgScore(scores[WHITE]) - MgScore(scores[BLACK]);
        egScore = EgScore(scores[WHITE]) - EgScore(scores[BLACK]);
    }
    //    printf("mgw=%d mgb=%d \n", get_mg_value(scores[WHITE]), get_mg_value(scores[BLACK]));
    //    printf("egw=%d egb=%d \n", get_eg_value(scores[WHITE]), get_eg_value(scores[BLACK]));


    int mgPhase = gamePhase;

    if (mgPhase > 24)
        mgPhase = 24; // in case of early promotion
    int egPhase = 24 - mgPhase;

    int score = (mgScore * mgPhase + egScore * egPhase) / 24;

    // Mise en cache de l'évaluation
    Transtable.store_evaluation(hash, score);

    //     printf("side=%d : mgp = %d ; egp = %d : s = %d \n", side_to_move, mgPhase, egPhase, score);

    // return score relative to the side to move
    //    return (side_to_move == WHITE ? score : -score) + Tempo;
    return (side_to_move == WHITE ? score : -score);
}


template int Board::evaluate<true>();
template int Board::evaluate<false>();
