#include "core/move_chain.h"

#include <utility>

#include "core/movegen.h"

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
  const MovePersistence persistence = moveMake(board_, move);
  moves_.push_back({move, persistence});
  doPush();
}

bool MoveChain::tryPush(const Move move) {
  const MovePersistence persistence = moveMake(board_, move);
  if (!isMoveLegal(board_)) {
    moveUnmake(board_, move, persistence);
    return false;
  }
  moves_.push_back({move, persistence});
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
