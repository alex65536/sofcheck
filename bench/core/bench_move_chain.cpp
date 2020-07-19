#include <benchmark/benchmark.h>

#include "core/bench_boards.h"
#include "core/board.h"
#include "core/init.h"
#include "core/move.h"
#include "core/move_chain.h"
#include "core/movegen.h"
#include "core/strutil.h"

using SoFCore::Board;
using SoFCore::cell_t;
using SoFCore::Move;
using SoFCore::MoveChain;
using SoFCore::MoveKind;

inline static Move createMove(const char *src, const char *dst, MoveKind kind = MoveKind::Simple,
                              cell_t promote = 0) {
  using SoFCore::charsToCoord;

  return Move{kind, charsToCoord(src[0], src[1]), charsToCoord(dst[0], dst[1]), promote};
}

MoveChain generateChainSicilian() {
  using namespace SoFCore;

  MoveChain chain(Board::initialPosition());
  chain.push(createMove("e2", "e4", MoveKind::PawnDoubleMove));
  chain.push(createMove("c7", "c5", MoveKind::PawnDoubleMove));
  chain.push(createMove("g1", "f3"));
  chain.push(createMove("d7", "d6"));
  chain.push(createMove("d2", "d4", MoveKind::PawnDoubleMove));
  chain.push(createMove("c5", "d4"));
  chain.push(createMove("f3", "d4"));
  chain.push(createMove("g1", "f6"));
  chain.push(createMove("b1", "c3"));
  chain.push(createMove("a7", "a6"));
  chain.push(createMove("c1", "g5"));
  chain.push(createMove("e7", "e6"));
  chain.push(createMove("f2", "f4", MoveKind::PawnDoubleMove));
  chain.push(createMove("f8", "e7"));
  chain.push(createMove("d1", "f3"));
  chain.push(createMove("d8", "c7"));
  chain.push(createMove("e1", "c1", MoveKind::CastlingQueenside));
  chain.push(createMove("b8", "d7"));
  chain.push(createMove("g2", "g4", MoveKind::PawnDoubleMove));
  chain.push(createMove("b7", "b5", MoveKind::PawnDoubleMove));

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
