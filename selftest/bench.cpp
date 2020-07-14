#include "chess_intf.h"

#include "benchmark.h"
#include <algorithm>
#include "util.h"

const char *g_fenInitial = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const char *g_fenSicilian = "r1b1k2r/2qnbppp/p2ppn2/1p4B1/3NPPP1/2N2Q2/PPP4P/2KR1B1R w kq - 0 11";
const char *g_fenMiddle = "1rq1r1k1/1p3ppp/pB3n2/3ppP2/Pbb1P3/1PN2B2/2P2QPP/R1R4K w - - 1 21";
const char *g_fenOpenPosition = "4r1k1/3R1ppp/8/5P2/p7/6PP/4pK2/1rN1B3 w - - 4 43";
const char *g_fenQueens = "6K1/8/8/1k3q2/3Q4/8/8/8 w - - 0 1";
const char *g_fenPawnsMove = "4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1";
const char *g_fenPawnsAttack = "4k3/8/8/pppppppp/PPPPPPPP/8/8/4K3 w - - 0 1";
const char *g_fenCydonia = "5K2/1N1N1N2/8/1N1N1N2/1n1n1n2/8/1n1n1n2/5k2 w - - 0 1";
const char *g_fenMax = "3Q4/1Q4Q1/4Q3/2Q4R/Q4Q2/3Q4/NR4Q1/kN1BB1K1 w - - 0 1";

inline void runGenMoves(benchmark::State &state, const char *fen) {
    using namespace ChessIntf;

    init();

    Board board;
    boardFromFen(board, fen);
    for (auto _ : state) {
        MoveList moves;
        generateMoves(board, moves);
        benchmark::DoNotOptimize(moves);
    }
}

#define BENCH_DO(name) \
    static void BM_GenMoves##name(benchmark::State &state) { \
        runGenMoves(state, g_fen##name); \
    } \
    BENCHMARK(BM_GenMoves##name);
#include "bench_xmacro.h"
#undef BENCH_DO

inline void runMakeMove(benchmark::State &state, const char *fen) {
    using namespace ChessIntf;

    init();

    Board board;
    boardFromFen(board, fen);
    MoveList moveList;
    generateMoves(board, moveList);

    Move moves[240];
    int cnt = getMoveCount(moveList);
    for (int i = 0; i < cnt; ++i) {
        moves[i] = getMove(moveList, i);
    }
    std::sort(moves, moves + cnt, [&](const Move &a, const Move &b) {
        return getMoveHash(board, a) < getMoveHash(board, b);
    });

    for (auto _ : state) {
        for (int i = 0; i < cnt; ++i) {
            MovePersistence p;
            makeMove(board, moves[i], p);
            unmakeMove(board, moves[i], p);
        }
    }
}

#define BENCH_DO(name) \
    static void BM_MakeMove##name(benchmark::State &state) { \
        runMakeMove(state, g_fen##name); \
    } \
    BENCHMARK(BM_MakeMove##name);
#include "bench_xmacro.h"
#undef BENCH_DO

inline void runIsAttacked(benchmark::State &state, const char *fen) {
    using namespace ChessIntf;

    init();

    Board board;
    boardFromFen(board, fen);

    for (auto _ : state) {
        for (bool color : {true, false}) {
            for (char y = '8'; y >= '1'; --y) {
                for (char x = 'a'; x <= 'h'; ++x) {
                    benchmark::DoNotOptimize(isAttacked(board, color, x, y));
                }
            }
        }
    }
}

#define BENCH_DO(name) \
    static void BM_IsAttacked##name(benchmark::State &state) { \
        runIsAttacked(state, g_fen##name); \
    } \
    BENCHMARK(BM_IsAttacked##name);
#include "bench_xmacro.h"
#undef BENCH_DO

inline void runKingAttack(benchmark::State &state, const char *fen) {
    using namespace ChessIntf;

    init();

    Board board;
    boardFromFen(board, fen);

    for (auto _ : state) {
        benchmark::DoNotOptimize(isOpponentKingAttacked(board));
    }
}

#define BENCH_DO(name) \
    static void BM_KingAttack##name(benchmark::State &state) { \
        runKingAttack(state, g_fen##name); \
    } \
    BENCHMARK(BM_KingAttack##name);
#include "bench_xmacro.h"
#undef BENCH_DO

void recurseSearch(ChessIntf::Board &board, int d) {
    using namespace ChessIntf;

    if (d == 0) {
        return;
    }

    MoveList moves;
    generateMoves(board, moves);
    int cnt = getMoveCount(moves);

    for (int i = 0; i < cnt; ++i) {
        const Move &move = getMove(moves, i);
        MovePersistence persistence;
        makeMove(board, move, persistence);
        if (!isOpponentKingAttacked(board)) {
            recurseSearch(board, d-1);
        }
        unmakeMove(board, move, persistence);
    }
}

inline void runRecurse(benchmark::State &state, const char *fen) {
    using namespace ChessIntf;

    init();

    Board board;
    boardFromFen(board, fen);

    for (auto _ : state) {
        int d = state.range(0);
        recurseSearch(board, d);
    }
}

#define BENCH_DO(name) \
    static void BM_Recurse##name(benchmark::State &state) { \
        runRecurse(state, g_fen##name); \
    } \
    BENCHMARK(BM_Recurse##name) \
        ->Arg(1)->Arg(2)->Arg(3)->Arg(4) \
        ->Unit(benchmark::kMillisecond);
#include "bench_xmacro.h"
#undef BENCH_DO
