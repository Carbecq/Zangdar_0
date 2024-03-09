#include "Tuner.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include "defines.h"
#include "Board.h"
#include "evaluate.h"

/*
  Gradient Decent Tuning for Chess Engines as described by Andrew Grant
  in his paper: Evaluation & Tuning in Chess Engines:

*
*   voir aussi : Drofa, Ethereal
*
*/


//===================================================
//! \brief  Constructeur
//---------------------------------------------------
Tuner::Tuner() noexcept
{
    Trace       = {};
    EmptyTrace  = {};

}

void Tuner::runTexelTuning()
{
    double baseParams[NTERMS][N_PHASES] = {{0}};
    double params[NTERMS][N_PHASES] = {{0}};
    double momentum[NTERMS][N_PHASES] = {{0}};
    double velocity[NTERMS][N_PHASES] = {{0}};
    double K;
    double error = 0;
    double rate = LRRATE;

    TexelEntry *entries = (TexelEntry*) calloc(NPOSITIONS, sizeof(TexelEntry));
    TupleStack          = (TexelTuple*) calloc(STACKSIZE,  sizeof(TexelTuple));

    printf("Tuning %d terms using %d positions from %s\n", NTERMS, NPOSITIONS, DATASET);

    // Initialisation des valeurs de base
    InitBaseParams(baseParams);
    PrintParameters(params, baseParams);

    // Initialisation du tuner
    InitTunerEntries(entries);

    printf("Optimal K...\r");
    K = ComputeOptimalK(entries);
    printf("Optimal K: %g\n\n", K);

    for (int epoch = 1; epoch <= MAXEPOCHS; epoch++) {

        double gradient[NTERMS][N_PHASES] = { {0} };
        ComputeGradient(entries, gradient, params, K);

        for (int i = 0; i < NTERMS; i++) 
        {
            double mg_grad = (-K / 200.0) * gradient[i][MG] / NPOSITIONS;
            double eg_grad = (-K / 200.0) * gradient[i][EG] / NPOSITIONS;

            momentum[i][MG] = BETA_1 * momentum[i][MG] + (1.0 - BETA_1) * mg_grad;
            momentum[i][EG] = BETA_1 * momentum[i][EG] + (1.0 - BETA_1) * eg_grad;

            velocity[i][MG] = BETA_2 * velocity[i][MG] + (1.0 - BETA_2) * pow(mg_grad, 2);
            velocity[i][EG] = BETA_2 * velocity[i][EG] + (1.0 - BETA_2) * pow(eg_grad, 2);

            params[i][MG] -= rate * momentum[i][MG] / (1e-8 + sqrt(velocity[i][MG]));
            params[i][EG] -= rate * momentum[i][EG] / (1e-8 + sqrt(velocity[i][EG]));
        }

        error = TunedEvaluationErrors(entries, params, K);
        printf("\rEpoch [%d] Error = [%.8f], Rate = [%g]", epoch, error, rate);

        // Pre-scheduled Learning Rate drops
        if (epoch % LRSTEPRATE == 0) rate = rate / LRDROPRATE;
        if (epoch % REPORTING == 0) PrintParameters(params, baseParams);
    }
}

//============================================================
//! \brief  Initalisation du tuner avec les valeurs
//! du DATASET
//------------------------------------------------------------
void Tuner::InitTunerEntries(TexelEntry* entries)
{
    Board board;

    std::string     str_file = DATASET;
    std::ifstream   file(str_file);

    // Ouverture du fichier
    if (!file.is_open())
    {
        std::cout << "[Tuner::InitTunerEntries] impossible d'ouvrir le fichier " << str_file << std::endl;
        exit(1);
    }
    else
    {
        std::cout << "[Tuner::InitTunerEntries] fichier " << str_file << " ouvert" << std::endl;
    }

    std::string     line;
    std::string     str_fen;
    std::string     str_res;
    std::string     str;
    std::string     aux;
    std::vector<std::string> liste;
    int  index = 0;
    bool f = true;

    // Lecture du fichier ligne à ligne
    while (std::getline(file, line))
    {
        // ligne vide
        if (line.size() < 3)
            continue;

        // Commentaire ou espace au début de la ligne
        aux = line.substr(0,1);
        if (aux == "/" || aux == " ")
            continue;

        liste = split(line, '[');           // Lichess
        // liste = split(line, '"');       // Zurichess

        str_fen = liste[0];
        str_res = liste[1];

        // std::cout << str_fen << std::endl;
        // std::cout << str_res << std::endl;


        f = true;

        // Lichess
        if (line.find("[1.0]") != std::string::npos)
            entries[index].result = 1.0;
        else if (line.find("[0.5]") != std::string::npos)
            entries[index].result = 0.5;
        else if (line.find("[0.0]") != std::string::npos)
            entries[index].result = 0.0;
        else
        {
            printf("Cannot Parse %s\n", line.c_str());
            f = false;
        }

        // Zurichess
        // if (line.find("1-0") != std::string::npos)
        //     entries[index].result = 1.0;
        // else if (line.find("1/2-1/2") != std::string::npos)
        //     entries[index].result = 0.5;
        // else if (line.find("0-1") != std::string::npos)
        //     entries[index].result = 0.0;
        // else
        // {
        //     printf("Cannot Parse %s\n", line.c_str());
        //     f = false;
        // }

        // if      (str_res == "1.0]") entries[i].result = 1.0;
        // else if (str_res == "0.5]") entries[i].result = 0.5;
        // else if (str_res == "0.0]") entries[i].result = 0.0;
        // else    {printf("Cannot Parse %s\n", line.c_str()); f = false; /*exit(EXIT_FAILURE);*/}

        if (f == true)
        {
            // Set the board with the current FEN and initialize
            board.set_fen(str_fen, false);
            InitTunerEntry(entries[index], &board);
            index++;
        }

        line = "";
        liste.clear();
    }

    file.close();
    if (index != NPOSITIONS)
    {
        std::cout << "erreur index " << index << "  positions " << NPOSITIONS << std::endl;
        return;
    }

    std::cout << "lecture effectuée" << std::endl;

    //   printf("%d %d %d \n", nbr, min_count, max_count);

}

//=======================================================================
//! \brief  Initialisation d'une entrée du tuner
//! Chaque entrée correspond à une position du dataset
//-----------------------------------------------------------------------
void Tuner::InitTunerEntry(TexelEntry& entry, Board *board)
{
    // phase calculée à partir du nombre de pièces
    int phase24 = board->get_phase24();

    // phase24 =  0  : EndGame
    //           24  : MiddleGame
    entry.pfactors[MG] = 0 + phase24 / 24.0;   // 0-24 -> 0-1 ; donc 1 en MG
    entry.pfactors[EG] = 1 - phase24 / 24.0;   // 0-24 -> 1-0 ; donc 1 en EG

    // phase
    entry.phase256 = board->get_phase256(phase24);

    double coeffs[NTERMS];
    Trace = EmptyTrace;

    // Save a WHITE POV static evaluation
    // Evaluation should be from White POV,
    // but we still call evaluate() from stm perspective
    // to get right tempo evaluation
    entry.seval = (board->side_to_move == WHITE) ? board->evaluate()
                                                 : -board->evaluate();

    // Initialisation des coefficients de "Trace"
    // à partir de l'évaluation
    InitCoefficients(coeffs);

    // Allocation mémoire des tuples de l'entrée
    // et initialisation à partir des coefficients "coeffs"
    InitTunerTuples(entry, coeffs);

    // 5. Save Final evaluation for easier gradient recalculation
    // As we called evaluate() from stm perspective
    // we need to adjust it here to be from WHITE POW
    entry.eval  = Trace.eval;
    entry.turn  = board->side_to_move;

    // 6. Also save modifiers to know is it is
    // OCBEndgame
    entry.scale = Trace.scale / 128.0;
}

void Tuner::InitTunerTuples(TexelEntry& entry, const double coeffs[NTERMS])
{
    static int allocs = 0;
    int length = 0, tidx = 0;

    // Count the needed Coefficients
    // un coeff nul signifie que les scores Blancs et Noirs sont égaux
    for (int i = 0; i < NTERMS; i++)
        length += coeffs[i] != 0.0;

    // TupleStack est alloué au départ, puis chaque entrée
    // va en utiliser une portion.
    // Lorsque tout le tableau TupleStack est utilisé, on refait
    // une allocation.
    // Cette méthode permet de limiter les allocations.

    /* |..........|...........|..............   TupleStack
     * |          |           |
     * entrée 1   |           |
     *            entrée 2    |
     *                        |
     *                        entrée 3
     */

    // Il ne reste plus assez de place dans TupleStack
    // Il faut re-allouer de la mémoire.
    if (length > TupleStackSize)
    {
        TupleStackSize = STACKSIZE;
        TupleStack = (TexelTuple*)calloc(STACKSIZE, sizeof(TexelTuple));    // nouvelle allocation
        int ttupleMB = STACKSIZE * sizeof(TexelTuple) / (1 << 20);
        printf("Allocating [%dMB] x%d\r", ttupleMB, ++allocs);
    }

    // Pointe l'entrée sur la portion de TupleStack
    // qui lui est destinée.
    entry.tuples   = TupleStack;    // pointe l'entrée sur sa zone de TupleStack
    entry.ntuples  = length;        // nombre d'éléments effectifs (non-nuls)
    TupleStack     += length;       // déplace le pointeur pour la prochaine entrée
    TupleStackSize -= length;       // diminue la place disponible restante de TupleStack

    // Initialise les éléments de tuples
    // i = index dans les coefficients
    for (int i = 0; i < NTERMS; i++)
        if (coeffs[i] != 0.0)
            entry.tuples[tidx++] = {i, coeffs[i]};


}

void Tuner::InitCoefficients(double coeffs[NTERMS])
{
    int index = 0;

    // Piece values
    InitCoeffSingle(coeffs, Trace.PawnValue,    index);
    InitCoeffSingle(coeffs, Trace.KnightValue,  index);
    InitCoeffSingle(coeffs, Trace.BishopValue,  index);
    InitCoeffSingle(coeffs, Trace.RookValue,    index);
    InitCoeffSingle(coeffs, Trace.QueenValue,   index);

    // PSQT
    InitCoeffArray(coeffs, Trace.PawnPSQT,      N_SQUARES, index);
    InitCoeffArray(coeffs, Trace.KnightPSQT,    N_SQUARES, index);
    InitCoeffArray(coeffs, Trace.BishopPSQT,    N_SQUARES, index);
    InitCoeffArray(coeffs, Trace.RookPSQT,      N_SQUARES, index);
    InitCoeffArray(coeffs, Trace.QueenPSQT,     N_SQUARES, index);
    InitCoeffArray(coeffs, Trace.KingPSQT,      N_SQUARES, index);

    // Pions
    InitCoeffSingle(coeffs, Trace.PawnDoubled,                  index);
    InitCoeffSingle(coeffs, Trace.PawnSupport,                  index);
    InitCoeffSingle(coeffs, Trace.PawnOpen,                     index);
    InitCoeffArray( coeffs, Trace.PawnPhalanx,      N_RANKS,    index);
    InitCoeffSingle(coeffs, Trace.PawnIsolated,                 index);
    InitCoeffArray( coeffs, Trace.PawnPassed,       N_RANKS,    index);
    InitCoeffArray( coeffs, Trace.PassedDefended,   N_RANKS,    index);

    // Pions passés
    InitCoeffArray( coeffs, Trace.PassedDistUs,     4,       index);
    InitCoeffSingle(coeffs, Trace.PassedDistThem,            index);
    InitCoeffArray( coeffs, Trace.PassedBlocked,    4,       index);

    // Divers
    InitCoeffSingle(coeffs, Trace.MinorBehindPawn,          index);
    InitCoeffArray( coeffs, Trace.KnightOutpost,    2,      index);
    InitCoeffSingle(coeffs, Trace.BishopPair,               index);
    InitCoeffSingle(coeffs, Trace.BishopBadPawn,            index);
    InitCoeffSingle(coeffs, Trace.OpenForward,              index);
    InitCoeffSingle(coeffs, Trace.SemiForward,              index);
    InitCoeffArray( coeffs, Trace.KingLineDanger,   28,     index);
    InitCoeffSingle(coeffs, Trace.KingAtkPawn,              index);

    // Menaces
    InitCoeffSingle(coeffs, Trace.PawnThreat,              index);
    InitCoeffSingle(coeffs, Trace.PushThreat,              index);

    // Mobilité
    InitCoeffArray(coeffs, Trace.KnightMobility,     9, index);
    InitCoeffArray(coeffs, Trace.BishopMobility,    14, index);
    InitCoeffArray(coeffs, Trace.RookMobility,      15, index);
    InitCoeffArray(coeffs, Trace.QueenMobility,     28, index);

    if (index != NTERMS){
        printf("Error in InitCoefficients(): i = %d ; NTERMS = %d\n", index, NTERMS);
        exit(EXIT_FAILURE);
    }
}

//===============================================================
//! \brief  Initialisation à partir des valeurs de "evaluate.h"
//---------------------------------------------------------------
void Tuner::InitBaseParams(double tparams[NTERMS][N_PHASES])
{
    int index = 0;

    // Piece values
    InitBaseSingle(tparams, PawnValue,      index);
    InitBaseSingle(tparams, KnightValue,    index);
    InitBaseSingle(tparams, BishopValue,    index);
    InitBaseSingle(tparams, RookValue,      index);
    InitBaseSingle(tparams, QueenValue,     index);

    // PSQT
    InitBaseArray(tparams, PawnPSQT,    N_SQUARES, index);
    InitBaseArray(tparams, KnightPSQT,  N_SQUARES, index);
    InitBaseArray(tparams, BishopPSQT,  N_SQUARES, index);
    InitBaseArray(tparams, RookPSQT,    N_SQUARES, index);
    InitBaseArray(tparams, QueenPSQT,   N_SQUARES, index);
    InitBaseArray(tparams, KingPSQT,    N_SQUARES, index);

    // Pions
    InitBaseSingle(tparams, PawnDoubled,             index);
    InitBaseSingle(tparams, PawnSupport,             index);
    InitBaseSingle(tparams, PawnOpen,                index);
    InitBaseArray( tparams, PawnPhalanx,    N_RANKS, index);
    InitBaseSingle(tparams, PawnIsolated,            index);
    InitBaseArray( tparams, PawnPassed,     N_RANKS, index);
    InitBaseArray( tparams, PassedDefended, N_RANKS, index);

    // Pions passés
    InitBaseArray( tparams, PassedDistUs,   4,       index);
    InitBaseSingle(tparams, PassedDistThem,          index);
    InitBaseArray( tparams, PassedBlocked,  4,       index);

    // Divers
    InitBaseSingle(tparams, MinorBehindPawn,        index);
    InitBaseArray( tparams, KnightOutpost,  2,      index);
    InitBaseSingle(tparams, BishopPair,             index);
    InitBaseSingle(tparams, BishopBadPawn,          index);
    InitBaseSingle(tparams, OpenForward,            index);
    InitBaseSingle(tparams, SemiForward,            index);
    InitBaseArray( tparams, KingLineDanger, 28,     index);
    InitBaseSingle(tparams, KingAtkPawn,            index);

    // Menaces
    InitBaseSingle(tparams, PawnThreat,              index);
    InitBaseSingle(tparams, PushThreat,              index);

    // Mobilité
    InitBaseArray( tparams, KnightMobility,  9,      index);
    InitBaseArray( tparams, BishopMobility, 14,      index);
    InitBaseArray( tparams, RookMobility,   15,      index);
    InitBaseArray( tparams, QueenMobility,  28,      index);

    if (index != NTERMS) {
        printf("Error 1 in InitBaseParams(): i = %d ; NTERMS = %d\n", index, NTERMS);
        exit(EXIT_FAILURE);
    }
}


double Tuner::Sigmoid(double K, double E)
{
    return 1.0 / (1.0 + exp(-K * E / 400.0));
}


double Tuner::ComputeOptimalK(const TexelEntry *entries)
{
    const double rate = 100, delta = 1e-5, deviation_goal = 1e-6;
    double K = 2, deviation = 1;

    while (fabs(deviation) > deviation_goal)
    {
        double up   = StaticEvaluationErrors(entries, K + delta);
        double down = StaticEvaluationErrors(entries, K - delta);
        deviation = (up - down) / (2 * delta);
        K -= deviation * rate;

        printf("up = %g ; down = %g ; deviation = %g ; goal = %g \n", up, down, fabs(deviation), deviation_goal);
    }

    return K;

}

double Tuner::StaticEvaluationErrors(const TexelEntry* entries, double K)
{
    // Compute the error of the dataset using the Static Evaluation.
    // We provide simple speedups that make use of the OpenMP Library.
    double total = 0.0;
#pragma omp parallel shared(total)
    {
#pragma omp for schedule(static, NPOSITIONS / NPARTITIONS) reduction(+:total)
        for (int i = 0; i < NPOSITIONS; i++)
            total += pow(entries[i].result - Sigmoid(K, entries[i].seval), 2);
    }

    return total / (double) NPOSITIONS;
}

double Tuner::LinearEvaluation(const TexelEntry& entry, double params[NTERMS][N_PHASES])
{
    double midgame = MgScore(entry.eval);
    double endgame = EgScore(entry.eval);

    // Save any modifications for MG or EG for each evaluation type
    for (int i = 0; i < entry.ntuples; i++)
    {
        int termIndex = entry.tuples[i].index;
        midgame += (double) entry.tuples[i].coeff * params[termIndex][MG];
        endgame += (double) entry.tuples[i].coeff * params[termIndex][EG];
    }

    double eval = ( midgame * entry.phase256
                +   endgame * (256.0 - entry.phase256) * entry.scale ) / 256.0;

    return eval + (entry.turn == WHITE ? Tempo : -Tempo);

}

void Tuner::UpdateSingleGradient(const TexelEntry& entry, double gradient[NTERMS][N_PHASES], double params[NTERMS][N_PHASES], double K)
{
    double E = LinearEvaluation(entry, params);
    double S = Sigmoid(K, E);
    double X = (entry.result - S) * S * (1 - S);

    double mgBase = X * entry.pfactors[MG];
    double egBase = X * entry.pfactors[EG];

    for (int i = 0; i < entry.ntuples; i++)
    {
        int    index = entry.tuples[i].index;
        double coeff = entry.tuples[i].coeff;

        gradient[index][MG] += mgBase * coeff;
        gradient[index][EG] += egBase * coeff * entry.scale;
    }

}

void Tuner::ComputeGradient(const TexelEntry *entries, double gradient[NTERMS][N_PHASES], double params[NTERMS][N_PHASES], double K)
{

    #pragma omp parallel shared(gradient)
    {
        double local[NTERMS][N_PHASES] = { {0} };

        #pragma omp for schedule(static, NPOSITIONS / NPARTITIONS)
        for (int i = 0; i < NPOSITIONS; i++)
            UpdateSingleGradient(entries[i], local, params, K);

        for (int i = 0; i < NTERMS; i++) {
            gradient[i][MG] += local[i][MG];
            gradient[i][EG] += local[i][EG];
        }
    }

}

double Tuner::TunedEvaluationErrors(const TexelEntry *entries, double params[NTERMS][N_PHASES], double K)
{
    double total = 0.0;

    #pragma omp parallel shared(total)
    {
        #pragma omp for schedule(static, NPOSITIONS / NPARTITIONS) reduction(+:total)
        for (int i = 0; i < NPOSITIONS; i++)
            total += pow(entries[i].result - Sigmoid(K, LinearEvaluation(entries[i], params)), 2);
    }

    return total / (double) NPOSITIONS;

}


void Tuner::PrintParameters(double params[NTERMS][N_PHASES], double current[NTERMS][N_PHASES])
{
    int len = 8;    // nombre d'éléments par ligne
    double tparams[NTERMS][N_PHASES];

    for (int i = 0; i < NTERMS; ++i) {
        tparams[i][MG] = round(params[i][MG] + current[i][MG]);
        tparams[i][EG] = round(params[i][EG] + current[i][EG]);
    }

    int index = 0;
    puts("\n");

    PrintPieceValues(tparams, index);
    PrintPSQT(tparams, index);

    puts("\n//----------------------------------------------------------");
    puts("// Pawn bonuses and maluses");
    PrintSingle("PawnDoubled",      tparams, index);
    PrintSingle("PawnSupport",      tparams, index);
    PrintSingle("PawnOpen",         tparams, index);
    PrintArray( "PawnPhalanx",      tparams, index,     N_RANKS,    "[N_RANKS]", len);
    PrintSingle("PawnIsolated",     tparams, index);
    PrintArray( "PawnPassed",       tparams, index,     N_RANKS,    "[N_RANKS]", len);
    PrintArray( "PassedDefended",   tparams, index,     N_RANKS,    "[N_RANKS]", len);

    puts("\n//----------------------------------------------------------");
    puts("// Pions passés");
    PrintArray( "PassedDistUs",     tparams, index,     4,      "[4]",  len);
    PrintSingle("PassedDistThem",   tparams, index);
    PrintArray( "PassedBlocked",    tparams, index,     4,      "[4]",  len);

    puts("\n//----------------------------------------------------------");
    puts("// Misc Bonus");
    PrintSingle("MinorBehindPawn",  tparams, index);
    PrintArray( "KnightOutpost",    tparams, index,     2,      "[2]",  len);
    PrintSingle("BishopPair",       tparams, index);
    PrintSingle("BishopBadPawn",    tparams, index);
    PrintSingle("OpenForward",      tparams, index);
    PrintSingle("SemiForward",      tparams, index);
    PrintArray( "KingLineDanger",   tparams, index,     28,     "[28]", len);
    PrintSingle("KingAtkPawn",      tparams, index);

    puts("\n//----------------------------------------------------------");
    puts("// Menaces");
    PrintSingle("PawnThreat",       tparams, index);
    PrintSingle("PushThreat",       tparams, index);

    PrintMobility(tparams, index);
    puts("");

    if (index != NTERMS) {
        printf("Error 2 in PrintParameters(): i = %d ; NTERMS = %d\n", index, NTERMS);
        exit(EXIT_FAILURE);
    }
}

void Tuner::PrintPieceValues(double params[NTERMS][N_PHASES], int& index)
{
    puts("\n//----------------------------------------------------------");
    puts("//  Valeur des pièces");
    puts("enum PieceValueEMG {");
    printf("    P_MG = %4d, P_EG = %4d,\n", (int) params[index][MG], (int) params[index][EG]); index++;
    printf("    N_MG = %4d, N_EG = %4d,\n", (int) params[index][MG], (int) params[index][EG]); index++;
    printf("    B_MG = %4d, B_EG = %4d,\n", (int) params[index][MG], (int) params[index][EG]); index++;
    printf("    R_MG = %4d, R_EG = %4d,\n", (int) params[index][MG], (int) params[index][EG]); index++;
    printf("    Q_MG = %4d, Q_EG = %4d,\n", (int) params[index][MG], (int) params[index][EG]); index++;
    printf("    K_MG = %4d, K_EG = %4d \n", 0, 0);
    puts("};");
}

void Tuner::PrintPSQT(double params[NTERMS][N_PHASES], int &index)
{
    int len = 8;

    //    puts("\n// Black's point of view - easier to read as it's not upside down");

    puts("\n//----------------------------------------------------------");
    puts("//  Bonus positionnel des pièces");
    puts("//  Du point de vue des Blancs - Mes tables sont comme ça");

    PrintArray("PawnPSQT",   params, index, N_SQUARES, "[N_SQUARES]", len);
    puts(" ");
    PrintArray("KnightPSQT", params, index, N_SQUARES, "[N_SQUARES]", len);
    puts(" ");
    PrintArray("BishopPSQT", params, index, N_SQUARES, "[N_SQUARES]", len);
    puts(" ");
    PrintArray("RookPSQT",   params, index, N_SQUARES, "[N_SQUARES]", len);
    puts(" ");
    PrintArray("QueenPSQT",  params, index, N_SQUARES, "[N_SQUARES]", len);
    puts(" ");
    PrintArray("KingPSQT",   params, index, N_SQUARES, "[N_SQUARES]", len);
}

void Tuner::PrintMobility(double params[NTERMS][N_PHASES], int& index)
{
    int len = 5;

    puts("\n//----------------------------------------------------------");
    printf("// Mobilité \n");

    PrintArray("KnightMobility", params, index,  9, "[9]",  len);
    puts(" ");
    PrintArray("BishopMobility", params, index, 14, "[14]", len);
    puts(" ");
    PrintArray("RookMobility",   params, index, 15, "[15]", len);
    puts(" ");
    PrintArray("QueenMobility",  params, index, 28, "[28]", len);
}

//     PrintSingle("PawnDoubled", tparams, i);

void Tuner::PrintSingle(const std::string& name, double params[NTERMS][N_PHASES], int& index)
{
    printf("constexpr Score %s = S(%4d, %4d);\n", name.c_str(), static_cast<int>(params[index][MG]), static_cast<int>(params[index][EG]));
    index++;
}

// PrintArray("PassedPawn", tparams, i, 8, "[N_RANKS]");   i+=8;

void Tuner::PrintArray(const std::string& name, double params[NTERMS][N_PHASES], int& index, int imax, const std::string& dim, int length)
{
    printf("constexpr Score %s%s = { ", name.c_str(), dim.c_str());

    for (int i = 0; i < imax; i++, index++)
    {
        if (i && i % length == 0)
            printf("\n    ");
        if (i == imax - 1)
            printf("S(%4d, %4d) ", static_cast<int>(params[index][MG]), static_cast<int>(params[index][EG]));
        else
            printf("S(%4d, %4d), ", static_cast<int>(params[index][MG]), static_cast<int>(params[index][EG]));
    }

    if (imax > length)
        printf("\n};\n");
    else
        printf("};\n");
}


//! \brief  Initialisation du tableau tparams à partir des données
//! provenant de "evaluate.h"
void Tuner::InitBaseSingle(double tparams[NTERMS][N_PHASES], const Score data, int& index)
{
    tparams[index][MG] = static_cast<double>(MgScore(data));
    tparams[index][EG] = static_cast<double>(EgScore(data));
    index++;
}

//! \brief  Initialisation d'un tableau[imax]
void Tuner::InitBaseArray(double tparams[NTERMS][N_PHASES], const Score* data, int imax, int& index)
{
    for (int i=0; i<imax; i++)
    {
        tparams[index][MG] = static_cast<double>(MgScore(data[i]));
        tparams[index][EG] = static_cast<double>(EgScore(data[i]));
        index++;
    }
}

//! \brief  Initialisation d'une valeur simple
//! à partir des données de Trace
void Tuner::InitCoeffSingle(double coeffs[NTERMS], Score score[2], int& index)
{
    coeffs[index++] = score[WHITE] - score[BLACK];
}

void Tuner::InitCoeffArray(double coeffs[NTERMS], Score score[][2], int imax, int& index)
{
    for (int i=0; i<imax; i++)
        coeffs[index++] = score[i][WHITE] - score[i][BLACK];
}
