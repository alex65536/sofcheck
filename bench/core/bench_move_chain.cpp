#include <benchmark/benchmark.h>

#include "core/bench_boards.h"
#include "core/board.h"
#include "core/init.h"
#include "core/move.h"
#include "core/move_chain.h"
#include "core/move_parser.h"
#include "core/movegen.h"
#include "core/strutil.h"

using SoFCore::Board;
using SoFCore::cell_t;
using SoFCore::Move;
using SoFCore::MoveChain;

MoveChain generateChainSicilian() {
  using namespace SoFCore;

  MoveChain chain(Board::initialPosition());
  chain.push(moveParse("e2e4", chain.position()));
  chain.push(moveParse("c7c5", chain.position()));
  chain.push(moveParse("g1f3", chain.position()));
  chain.push(moveParse("d7d6", chain.position()));
  chain.push(moveParse("d2d4", chain.position()));
  chain.push(moveParse("c5d4", chain.position()));
  chain.push(moveParse("f3d4", chain.position()));
  chain.push(moveParse("g8f6", chain.position()));
  chain.push(moveParse("b1c3", chain.position()));
  chain.push(moveParse("a7a6", chain.position()));
  chain.push(moveParse("c1g5", chain.position()));
  chain.push(moveParse("e7e6", chain.position()));
  chain.push(moveParse("f2f4", chain.position()));
  chain.push(moveParse("f8e7", chain.position()));
  chain.push(moveParse("d1f3", chain.position()));
  chain.push(moveParse("d8c7", chain.position()));
  chain.push(moveParse("e1c1", chain.position()));
  chain.push(moveParse("b8d7", chain.position()));
  chain.push(moveParse("g2g4", chain.position()));
  chain.push(moveParse("b7b5", chain.position()));

  return chain;
}

void recurseSearch(Board &board, int d) {
  using namespace SoFCore;

  if (d == 0) {
    return;
  }
  Move moves[240];
  size_t cnt = genAllMoves(board, moves);
  for (size_t i = 0; i < cnt; ++i) {
    const Move &move = moves[i];
    MovePersistence persistence = moveMake(board, move);
    if (isMoveLegal(board)) {
      recurseSearch(board, d - 1);
    }
    moveUnmake(board, move, persistence);
  }
}

void moveChainSearch(MoveChain &chain, int d) {
  using namespace SoFCore;

  if (d == 0) {
    return;
  }
  Move moves[240];
  size_t cnt = genAllMoves(chain.position(), moves);
  for (size_t i = 0; i < cnt; ++i) {
    if (!chain.tryPush(moves[i])) {
      continue;
    }
    moveChainSearch(chain, d - 1);
    chain.pop();
  }
}

void BM_RecurseSearch(benchmark::State &state) {
  SoFCore::init();

  MoveChain chain = generateChainSicilian();

  for (auto _ : state) {
    (void)_;
    recurseSearch(chain.position(), state.range(0));
  }
}
BENCHMARK(BM_RecurseSearch)->Arg(1)->Arg(2)->Arg(3)->Arg(4)->Unit(benchmark::kMillisecond);

void BM_MoveChainSearch(benchmark::State &state) {
  SoFCore::init();

  MoveChain chain = generateChainSicilian();

  for (auto _ : state) {
    (void)_;
    moveChainSearch(chain, state.range(0));
  }
}
BENCHMARK(BM_MoveChainSearch)->Arg(1)->Arg(2)->Arg(3)->Arg(4)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
