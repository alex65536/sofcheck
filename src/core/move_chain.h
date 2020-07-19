#ifndef SOF_CORE_MOVE_CHAIN_INCLUDED
#define SOF_CORE_MOVE_CHAIN_INCLUDED

#include <unordered_map>
#include <vector>

#include "core/board.h"
#include "core/move.h"
#include "core/types.h"

namespace SoFCore {

constexpr size_t REPETITIONS_FOR_DRAW = 3;

class MoveChain {
public:
  MoveChain(const Board &rootPosition);

  inline Board &position() { return board_; }
  const Board &position() const { return board_; }

  inline bool isEmpty() const { return moves_.empty(); }

  void push(Move move);
  bool tryPush(Move move);
  Move pop();

  bool isDrawByRepetition() const { return repeatedPositions_; }

private:
  struct State {
    Move move;
    MovePersistence persistence;
  };

  void doPush();

  Board board_;
  std::vector<State> moves_;
  std::unordered_map<board_hash_t, size_t> repetitions_;
  size_t repeatedPositions_;
};

}  // namespace SoFCore

#endif  // SOF_CORE_MOVE_CHAIN_INCLUDED
