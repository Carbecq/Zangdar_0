#ifndef TUNER_H
#define TUNER_H
#include "Board.h"

// Le fichier doit avoir les fins de linue en UNIX
//                 être UTF8 simple

// https://www.talkchess.com/forum3/viewtopic.php?f=7&t=75350


// #define DATASET "D:/Echecs/Programmation/Zangdar/tuner/AndrewGrant/E12.41-1M-D12-Resolved.book"
// #define NPOSITIONS   (9996883) // Total FENS in the book

// #define DATASET "D:/Echecs/Programmation/Zangdar/tuner/AndrewGrant/E12.33-12.41-1M-D12-Resolved.book"
// #define NPOSITIONS   (19996623) // Total FENS in the book

// #define DATASET "D:/Echecs/Programmation/Zangdar/tuner/AndrewGrant/E12.52-1M-D12-Resolved.book"
// #define NPOSITIONS   (9998762) // Total FENS in the book






struct EvalTrace {
    Score eval;
    int   scale;

    Score PawnValue[N_COLORS];
    Score KnightValue[N_COLORS];
    Score BishopValue[N_COLORS];
    Score RookValue[N_COLORS];
    Score QueenValue[N_COLORS];

    Score PawnPSQT[N_SQUARES][N_COLORS];
    Score KnightPSQT[N_SQUARES][N_COLORS];
    Score BishopPSQT[N_SQUARES][N_COLORS];
    Score RookPSQT[N_SQUARES][N_COLORS];
    Score QueenPSQT[N_SQUARES][N_COLORS];
    Score KingPSQT[N_SQUARES][N_COLORS];

    Score PawnDoubled[N_COLORS];
    Score PawnSupport[N_COLORS];
    Score PawnOpen[N_COLORS];
    Score PawnPhalanx[N_RANKS][N_COLORS];
    Score PawnIsolated[N_COLORS];
    Score PawnPassed[N_RANKS][N_COLORS];
    Score PassedDefended[N_RANKS][N_COLORS];

    Score PassedDistUs[N_RANKS][N_COLORS];
    Score PassedDistThem[N_COLORS];
    Score PassedBlocked[N_RANKS][N_COLORS];

    Score MinorBehindPawn[N_COLORS];
    Score KnightOutpost[2][N_COLORS];
    Score BishopPair[N_COLORS];
    Score BishopBadPawn[N_COLORS];
    Score OpenForward[N_COLORS];
    Score SemiForward[N_COLORS];
    Score KingLineDanger[28][N_COLORS];
    Score KingAtkPawn[N_COLORS];

    Score PawnThreat[N_COLORS];
    Score PushThreat[N_COLORS];

    Score KnightMobility[9][N_COLORS];
    Score BishopMobility[14][N_COLORS];
    Score RookMobility[15][N_COLORS];
    Score QueenMobility[28][N_COLORS];

};

struct TexelTuple {
    int    index;
    double coeff;
} ;


struct TexelEntry {
    int    seval;       // static eval
    int    phase256;
    int    turn;
    int    ntuples;
    Score  eval;
    double result;
    double scale;
    double pfactors[2];
    TexelTuple *tuples;
} ;




class Tuner
{
public:
    [[nodiscard]] Tuner() noexcept;
    void runTexelTuning();

    EvalTrace Trace;    // Paramètres initialisés lors de l'évaluation

private:
    // constexpr static char DATASET[] = "D:/Echecs/Programmation/Zangdar/tuner/Zurichess/quiet-labeled.epd";
    // constexpr static int NPOSITIONS = 725000; // Total FENS in the book

    // constexpr static char DATASET[] = "D:/Echecs/Programmation/Zangdar/tuner/AndrewGrant/E12.33-1M-D12-Resolved.book";
    // constexpr static int    NPOSITIONS    = 9999740; // Total FENS in the book

    constexpr static char DATASET[] = "D:/Echecs/Programmation/Zangdar/tuner/Lichess/lichess-big3-resolved.book";
    constexpr static int    NPOSITIONS    =  7153653; // Total FENS in the book

    constexpr static double NPOSITIONS_d  = NPOSITIONS; // Total FENS in the book

constexpr static int    N_PHASES    = 2;
constexpr static int    NTERMS      = 530;     // Number of terms being tuned
constexpr static double NTERMS_d    = 530;     // Number of terms being tuned

constexpr static int    MAXEPOCHS   =   10000; // Max number of epochs allowed
constexpr static int    REPORTING   =     100; // How often to print the new parameters
constexpr static int    NPARTITIONS =      64; // Total thread partitions
constexpr static double LRRATE      =    0.01; // Learning rate
constexpr static double LRDROPRATE  =    1.00; // Cut LR by this each LR-step
constexpr static int    LRSTEPRATE  =     250; // Cut LR after this many epochs
constexpr static double BETA_1      =     0.9; // ADAM Momemtum Coefficient
constexpr static double BETA_2      =   0.999; // ADAM Velocity Coefficient

constexpr static int    STACKSIZE   = static_cast<int>(NPOSITIONS_d * NTERMS_d / 64.0);

    TexelTuple* TupleStack;
    int TupleStackSize = STACKSIZE;
    EvalTrace EmptyTrace;

    double TexelCoeffs[NTERMS];
    double TexelVector[NTERMS][N_PHASES];

    double TunedEvaluationErrors(const TexelEntry *entries, double params[NTERMS][N_PHASES], double K);
    void   ComputeGradient(const TexelEntry *entries, double gradient[NTERMS][N_PHASES], double params[NTERMS][N_PHASES], double K);
    void   UpdateSingleGradient(const TexelEntry &entry, double gradient[NTERMS][N_PHASES], double params[NTERMS][N_PHASES], double K);
    double LinearEvaluation(const TexelEntry &entry, double params[NTERMS][N_PHASES]);
    double ComputeOptimalK(const TexelEntry *entries);
    double StaticEvaluationErrors(const TexelEntry *entries, double K);
    double Sigmoid(double K, double E);

    void   InitTunerEntries(TexelEntry *entries);
    void   InitTunerEntry(TexelEntry &entry, Board *board);
    void   InitTunerTuples(TexelEntry &entry, const double coeffs[NTERMS]);

    void   InitCoefficients(double coeffs[NTERMS]);
    void   InitCoeffSingle(double coeffs[NTERMS], Score score[2], int& index);
    void   InitCoeffArray(double coeffs[NTERMS], Score score[][2], int imax, int& index);

    void   PrintParameters(double params[NTERMS][N_PHASES], double current[NTERMS][N_PHASES]);
    void   PrintPieceValues(double params[NTERMS][N_PHASES], int &index);
    void   PrintPSQT(double params[NTERMS][N_PHASES], int& index);
    void   PrintMobility(double params[NTERMS][N_PHASES], int &index);


    void   InitBaseParams(double tparams[NTERMS][N_PHASES]);
    void   InitBaseSingle(double tparams[NTERMS][N_PHASES], const Score data, int& index);
    void   InitBaseArray(double tparams[NTERMS][N_PHASES], const Score* data, int imax, int& index);

    void   PrintArray(const std::string &name, double params[NTERMS][N_PHASES], int &index, int imax, const std::string &dim, int length);
    void   PrintSingle(const std::string &name, double params[NTERMS][N_PHASES], int &index);


};

extern Tuner ownTuner;

#endif // TUNER_H
