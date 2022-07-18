// This file is part of SoFCheck
//
// Copyright (c) 2020-2022 Alexander Kernozhitsky and SoFCheck contributors
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

#include <algorithm>

#include "chess_intf.h"
#include "core/bench_boards.h"
#include "selftest_config.h"
#include "util.h"

inline void runGenMoves(benchmark::State &state, const char *fen) {
  using namespace ChessIntf;

  init();

  Board board = boardFromFen(fen);
  for ([[maybe_unused]] auto _ : state) {
    MoveList moves = generateMoves(board);
    benchmark::DoNotOptimize(moves);
  }
}

#define BENCH_DO(name)                                                                        \
  static void BM_GenMoves##name(benchmark::State &state) { runGenMoves(state, g_fen##name); } \
  BENCHMARK(BM_GenMoves##name);
#include "core/bench_xmacro.h"
#undef BENCH_DO

inline void runMakeMove(benchmark::State &state, const char *fen) {
  using namespace ChessIntf;

  init();

  Board board = boardFromFen(fen);
  MoveList moveList = generateMoves(board);

  Move moves[MOVES_MAX];
  int cnt = getMoveCount(moveList);
  for (int i = 0; i < cnt; ++i) {
    moves[i] = getMove(moveList, i);
  }
  std::sort(moves, moves + cnt, [&](const Move &a, const Move &b) {
    return getMoveHash(board, a) < getMoveHash(board, b);
  });

  for ([[maybe_unused]] auto _ : state) {
    for (int i = 0; i < cnt; ++i) {
      MovePersistence p = makeMove(board, moves[i]);
      unmakeMove(board, moves[i], p);
    }
  }
}

#define BENCH_DO(name)                                                                        \
  static void BM_MakeMove##name(benchmark::State &state) { runMakeMove(state, g_fen##name); } \
  BENCHMARK(BM_MakeMove##name);
#include "core/bench_xmacro.h"
#undef BENCH_DO

#ifdef ATTACK_HEATMAPS
inline void runIsAttacked(benchmark::State &state, const char *fen) {
  using namespace ChessIntf;

  init();

  Board board = boardFromFen(fen);

  for ([[maybe_unused]] auto _ : state) {
    for (bool color : {true, false}) {
      for (char y = '8'; y >= '1'; --y) {
        for (char x = 'a'; x <= 'h'; ++x) {
          benchmark::DoNotOptimize(isAttacked(board, color, x, y));
        }
      }
    }
  }
}

#define BENCH_DO(name)                                                                            \
  static void BM_IsAttacked##name(benchmark::State &state) { runIsAttacked(state, g_fen##name); } \
  BENCHMARK(BM_IsAttacked##name);
#include "core/bench_xmacro.h"
#undef BENCH_DO

#endif

inline void runIsCheck(benchmark::State &state, const char *fen) {
  using namespace ChessIntf;

  init();

  Board board = boardFromFen(fen);

  for ([[maybe_unused]] auto _ : state) {
    benchmark::DoNotOptimize(isInCheck(board));
  }
}

#define BENCH_DO(name)                                                                            \
  static void BM_IsCheck##name(benchmark::State &state) { runIsCheck(state, g_fen##name); } \
  BENCHMARK(BM_IsCheck##name);
#include "core/bench_xmacro.h"
#undef BENCH_DO

void recurseSearch(ChessIntf::Board &board, int d) {
  using namespace ChessIntf;

  if (d == 0) {
    return;
  }

  MoveList moves = generateMoves(board);
  int cnt = getMoveCount(moves);

  for (int i = 0; i < cnt; ++i) {
    const Move &move = getMove(moves, i);
    MovePersistence persistence = makeMove(board, move);
    if (isLastMoveLegal(board)) {
      recurseSearch(board, d - 1);
    }
    unmakeMove(board, move, persistence);
  }
}

inline void runRecurse(benchmark::State &state, const char *fen) {
  using namespace ChessIntf;

  init();

  Board board = boardFromFen(fen);

  for ([[maybe_unused]] auto _ : state) {
    auto d = static_cast<int>(state.range(0));
    recurseSearch(board, d);
  }
}

#define BENCH_DO(name)                                                                      \
  static void BM_Recurse##name(benchmark::State &state) { runRecurse(state, g_fen##name); } \
  BENCHMARK(BM_Recurse##name)->Arg(1)->Arg(2)->Arg(3)->Arg(4)->Unit(benchmark::kMillisecond);
#include "core/bench_xmacro.h"
#undef BENCH_DO
