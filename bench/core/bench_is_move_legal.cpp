#include <benchmark/benchmark.h>

#include "core/bench_boards.h"
#include "core/board.h"
#include "core/init.h"
#include "core/movegen.h"
#include "util/misc.h"

inline void runCheckValid(benchmark::State &state, const char *fen) {
  using namespace SoFCore;

  init();

  Board board = Board::fromFen(fen).unwrap();
  Move moves[240];
  size_t cnt = genAllMoves(board, moves);

  for (auto _ : state) {
    unused(_);
    for (size_t i = 0; i < cnt; ++i) {
      benchmark::DoNotOptimize(isMoveValid(board, moves[i]));
    }
  }
}

#define BENCH_DO(name)                                                                            \
  static void BM_CheckValid##name(benchmark::State &state) { runCheckValid(state, g_fen##name); } \
  BENCHMARK(BM_CheckValid##name);
#include "core/bench_xmacro.h"
#undef BENCH_DO

BENCHMARK_MAIN();
