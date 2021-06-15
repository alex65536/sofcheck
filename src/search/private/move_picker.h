// This file is part of SoFCheck
//
// Copyright (c) 2020-2021 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_SEARCH_PRIVATE_MOVE_PICKER_INCLUDED
#define SOF_SEARCH_PRIVATE_MOVE_PICKER_INCLUDED

#include <algorithm>
#include <cstddef>

#include "core/move.h"
#include "core/movegen.h"
#include "util/operators.h"

namespace SoFCore {
struct Board;
}  // namespace SoFCore

namespace SoFSearch::Private {

// Types of moves that can be returned by `MovePicker`. This enumeration represents different stages
// of move sorting.
enum class MovePickerStage {
  Start = 0,
  HashMove = 1,
  Capture = 2,
  SimplePromote = 3,
  Killer = 4,
  History = 5,
  End = 6
};

SOF_ENUM_COMPARE(MovePickerStage, int)

class KillerLine;
class HistoryTable;

// Iterates over all the pseudo-legal moves in a given position. The moves arrive in an order which
// is good for alpha-beta search.
class MovePicker {
public:
  // Returns the type of the last move returned by the last call of `next()`. See `MovePickerStage`
  // description for more details.
  inline MovePickerStage stage() const { return stage_; }

  // Returns the next move. If the move is equal to `Move::invalid()`, then there are no moves left.
  // If the move is equal to `Move::null()`, then it must be skipped.
  inline SoFCore::Move next() {
    using SoFCore::Move;
    if (movePosition_ == moveCount_) {
      nextStage();
    }
    const Move move = moves_[movePosition_++];
    return (stage_ != MovePickerStage::HashMove && move == hashMove_) ? Move::null() : move;
  }

  MovePicker(const SoFCore::Board &board, const SoFCore::Move hashMove, const KillerLine &killers,
             const HistoryTable &history)
      : stage_(MovePickerStage::Start),
        hashMove_(hashMove),
        board_(board),
        killers_(killers),
        history_(history),
        savedKillers_{SoFCore::Move::null(), SoFCore::Move::null()},
        moveCount_(0),
        movePosition_(0) {}

private:
  void nextStage();

  MovePickerStage stage_;
  SoFCore::Move hashMove_;
  const SoFCore::Board &board_;
  const KillerLine &killers_;
  const HistoryTable &history_;
  SoFCore::Move moves_[SoFCore::BUFSZ_MOVES];
  SoFCore::Move savedKillers_[2];
  size_t moveCount_;
  size_t movePosition_;
};

// Iterates over all the moves that must be considered in quiescense search. The moves arrive in a
// "good" order, i.e. the order to make the quiescense search work faster.
class QuiescenseMovePicker {
public:
  // Returns the next move. If the move is equal to `Move::invalid()`, then there are no moves left.
  // If the move is equal to `Move::null()`, then it must be skipped.
  inline SoFCore::Move next() {
    if (movePosition_ == moveCount_) {
      if (stage_ == Stage::Capture) {
        stage_ = Stage::SimplePromote;
        addSimplePromotes();
      }
      if (movePosition_ == moveCount_) {
        return SoFCore::Move::invalid();
      }
    }
    return moves_[movePosition_++];
  }

  explicit QuiescenseMovePicker(const SoFCore::Board &board);

private:
  enum class Stage { Capture, SimplePromote };

  void addSimplePromotes();

  const SoFCore::Board &board_;
  SoFCore::Move moves_[std::max(SoFCore::BUFSZ_CAPTURES, SoFCore::BUFSZ_SIMPLE_PROMOTES)];
  size_t moveCount_;
  size_t movePosition_;
  Stage stage_;
};

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_MOVE_PICKER_INCLUDED
