#include "core/move_chain.h"

#include <utility>

#include "core/movegen.h"
#include "util/misc.h"

namespace SoFCore {

MoveChain::MoveChain(const Board &rootPosition) : board_(rootPosition), repeatedPositions_(0) {
  ++repetitions_[board_.hash];
}

void MoveChain::doPush() {
  size_t &curRepetitions = repetitions_[board_.hash];
  ++curRepetitions;
  if (curRepetitions == REPETITIONS_FOR_DRAW) {
    ++repeatedPositions_;
  }
}

void MoveChain::push(const Move move) {
  moves_.emplace_back();
  moves_.back().move = move;
  moves_.back().persistence = moveMake(board_, move);
  doPush();
}

bool MoveChain::tryPush(const Move move) {
  moves_.emplace_back();
  moves_.back().persistence = moveMake(board_, move);
  if (unlikely(!isMoveLegal(board_))) {
    moveUnmake(board_, move, moves_.back().persistence);
    moves_.pop_back();
    return false;
  }
  moves_.back().move = move;
  doPush();
  return true;
}

Move MoveChain::pop() {
  size_t &curRepetitions = repetitions_[board_.hash];
  if (curRepetitions == REPETITIONS_FOR_DRAW) {
    --repeatedPositions_;
  }
  --curRepetitions;
  if (curRepetitions == 0) {
    repetitions_.erase(board_.hash);
  }
  const Move move = moves_.back().move;
  moveUnmake(board_, move, moves_.back().persistence);
  moves_.pop_back();
  return move;
}

}  // namespace SoFCore
