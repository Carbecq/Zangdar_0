#include "Board.h"
#include "defines.h"
#include "evaluate.h"

//==========================================
//! \brief  Evaluation de la position
//------------------------------------------
template<bool Mode>
[[nodiscard]] int Board::evaluate()
{
    int gamePhase = 0;
    int mgScore;
    int egScore;

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


    int phase_256 = std::max(0, 24 - gamePhase);
//    if (phase_256 < 0.0)
//        phase_256 = 0.0; // in case of early promotion

    phase_256 = (phase_256 * 256 + 12) / 24;
    int eval_256  = (mgScore * (256 - phase_256) + egScore * phase_256) / 256;

//    int phase_24 = std::min(24, gamePhase);
//    if (phase_24 > 24.0)
//        phase_24 = 24.0; // in case of early promotion

//    int eval_24 = (mgScore * phase_24 + egScore * (24 - phase_24)) / 24;

    // eval_24 et eval_256 sont égaux à +- 1
    // mais eval_24 donne une évaluation plus rapidement

//    if (eval_24 != eval_256)
//   printf("phase = %f %f : eval = %f %f \n", phase_24, phase_256, eval_24, eval_256);

    //     printf("side=%d : mgp = %d ; egp = %d : s = %d \n", side_to_move, mgPhase, egPhase, score);

    // return score relative to the side to move
    return (side_to_move == WHITE ? eval_256 : -eval_256) + Tempo;
}

template int Board::evaluate<true>();
template int Board::evaluate<false>();
