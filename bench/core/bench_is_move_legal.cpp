// This file is part of SoFCheck
//
// Copyright (c) 2020-2021, 2023 Alexander Kernozhitsky and SoFCheck contributors
//
// SoFCheck is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SoFCheck is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

#include <benchmark/benchmark.h>

#include <cstddef>

#include "core/bench_boards.h"
#include "core/board.h"
#include "core/init.h"
#include "core/move.h"
#include "core/movegen.h"
#include "core/types.h"
#include "util/result.h"

inline void runCheckValid(benchmark::State &state, const char *fen) {
  using namespace SoFCore;

  init();

  Board board = Board::fromFen(fen).unwrap();
  Move moves[BUFSZ_MOVES];
  size_t cnt = MoveGen(board).genAllMoves(moves);

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
