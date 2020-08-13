#ifndef SOF_SEARCH_PRIVATE_MOVE_PICKER_INCLUDED
#define SOF_SEARCH_PRIVATE_MOVE_PICKER_INCLUDED

#include "core/board.h"
#include "core/move.h"
#include "search/private/types.h"

namespace SoFSearch::Private {

using SoFCore::Board;
using SoFCore::Move;

// Types of moves that can be returned by `MovePicker`. This enumeration represents different stages
// of move sorting.
enum class MovePickerStage {
  Start = 0,
  HashMove = 1,
  Capture = 2,
  Killer = 3,
  History = 4,
  End = 5
};

// Comparison operators for `MovePickerStage`
#define D_MOVEPICKER_STAGE_COMPARE_OP(op)                                     \
  inline constexpr bool operator op(MovePickerStage l1, MovePickerStage l2) { \
    return static_cast<int>(l1) op static_cast<int>(l2);                      \
  }

D_MOVEPICKER_STAGE_COMPARE_OP(<)
D_MOVEPICKER_STAGE_COMPARE_OP(<=)
D_MOVEPICKER_STAGE_COMPARE_OP(>)
D_MOVEPICKER_STAGE_COMPARE_OP(>=)
D_MOVEPICKER_STAGE_COMPARE_OP(==)
D_MOVEPICKER_STAGE_COMPARE_OP(!=)

#undef D_MOVEPICKER_STAGE_COMPARE_OP

// Iterates over all the pseudo-legal moves in a given position. The moves arrive in an order which
// is good for alpha-beta search.
class MovePicker {
public:
  // Returns the type of the last move returned by the last call of `next()`. See `MovePickerStage`
  // description for more details.
  inline MovePickerStage stage() const { return stage_; }

  // Returns the next move. If the move is equal to `Move::invalid()`, then there are no moves left.
  // If the move is equal to `Move::null()`, then it must be skipped.
  inline Move next() {
    if (movePosition_ == moveCount_) {
      nextStage();
    }
    const Move move = moves_[movePosition_++];
    return (stage_ != MovePickerStage::HashMove && move == hashMove_) ? Move::null() : move;
  }

  MovePicker(const Board &board, const Move hashMove, const KillerLine &killers,
             const HistoryTable &history)
      : stage_(MovePickerStage::Start),
        hashMove_(hashMove),
        board_(board),
        killers_(killers),
        history_(history),
        savedKillers_{Move::null(), Move::null()},
        moveCount_(0),
        movePosition_(0) {}

private:
  void nextStage();

  MovePickerStage stage_;
  Move hashMove_;
  const Board &board_;
  const KillerLine &killers_;
  const HistoryTable &history_;
  Move moves_[256];
  Move savedKillers_[2];
  size_t moveCount_;
  size_t movePosition_;
};

// Iterates over all the moves that must be considered in quiescense search. The moves arrive in a
// "good" order, i.e. the order to make the quiescense search work faster.
class QuiescenseMovePicker {
public:
  // Returns the next move. If the move is equal to `Move::invalid()`, then there are no moves left.
  // If the move is equal to `Move::null()`, then it must be skipped.
  inline Move next() {
    if (movePosition_ == moveCount_) {
      return Move::invalid();
    }
    return moves_[movePosition_++];
  }

  QuiescenseMovePicker(const Board &board);

private:
  Move moves_[128];
  size_t moveCount_;
  size_t movePosition_;
};

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_MOVE_PICKER_INCLUDED
