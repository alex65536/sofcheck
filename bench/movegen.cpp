#include <benchmark/benchmark.h>

#include "core/board.h"
#include "core/types.h"
#include "core/movegen.h"

#include <iostream>

static void BM_AttackSimpleArrs(benchmark::State &state) {
  using namespace SoFCore;
  
  Board board;
  board.setFromFen("5n2/ppKN4/4PP1n/1N2N1P1/4k3/1P4p1/n7/3n1N2 w - - 0 1");
  
  for (auto _ : state) {
    for (coord_t i = 0; i < 64; ++i) {
      benchmark::DoNotOptimize(isCellAttacked_simpleArrs<Color::White>(board, i));
      benchmark::DoNotOptimize(isCellAttacked_simpleArrs<Color::Black>(board, i));
    }
  }
}
BENCHMARK(BM_AttackSimpleArrs);

static void BM_AttackSimple(benchmark::State &state) {
  using namespace SoFCore;
  
  Board board;
  board.setFromFen("5n2/ppKN4/4PP1n/1N2N1P1/4k3/1P4p1/n7/3n1N2 w - - 0 1");
  
  for (auto _ : state) {
    for (coord_t i = 0; i < 64; ++i) {
      benchmark::DoNotOptimize(isCellAttacked_simple<Color::White>(board, i));
      benchmark::DoNotOptimize(isCellAttacked_simple<Color::Black>(board, i));
    }
  }
}
BENCHMARK(BM_AttackSimple);

static void BM_AttackSse(benchmark::State &state) {
  using namespace SoFCore;
  
  Board board;
  board.setFromFen("5n2/ppKN4/4PP1n/1N2N1P1/4k3/1P4p1/n7/3n1N2 w - - 0 1");
  
  for (auto _ : state) {
    for (coord_t i = 0; i < 64; ++i) {
      benchmark::DoNotOptimize(isCellAttacked_simple<Color::White>(board, i));
      benchmark::DoNotOptimize(isCellAttacked_simple<Color::Black>(board, i));
    }
  }
}
BENCHMARK(BM_AttackSse);

static void BM_AttackAvx(benchmark::State &state) {
  using namespace SoFCore;
  
  Board board;
  board.setFromFen("5n2/ppKN4/4PP1n/1N2N1P1/4k3/1P4p1/n7/3n1N2 w - - 0 1");
  
  for (auto _ : state) {
    for (coord_t i = 0; i < 64; ++i) {
      benchmark::DoNotOptimize(isCellAttacked_avx<Color::White>(board, i));
      benchmark::DoNotOptimize(isCellAttacked_avx<Color::Black>(board, i));
    }
  }
}
BENCHMARK(BM_AttackAvx);

BENCHMARK_MAIN();
