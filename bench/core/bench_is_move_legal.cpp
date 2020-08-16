#include <benchmark/benchmark.h>

#include "core/bench_boards.h"
#include "core/board.h"
#include "core/init.h"
#include "core/movegen.h"

inline void runCheckValid(benchmark::State &state, const char *fen) {
  using namespace SoFCore;

  init();

  Board board = Board::fromFen(fen).unwrap();
  Move moves[BUFSZ_MOVES];
  size_t cnt = genAllMoves(board, moves);

  for ([[maybe_unused]] auto _ : state) {
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
