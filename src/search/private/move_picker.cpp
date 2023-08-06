// This file is part of SoFCheck
//
// Copyright (c) 2020-2023 Alexander Kernozhitsky and SoFCheck contributors
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

#include "search/private/move_picker.h"

#include <algorithm>
#include <cstdint>

#include "core/board.h"
#include "core/types.h"
#include "search/private/util.h"
#include "util/misc.h"

namespace SoFSearch::Private {

using SoFCore::Board;
using SoFCore::Move;

void sortMvvLva(const Board &board, Move *moves, const size_t count) {
  constexpr uint8_t victimOrd[16] = {8, 8, 0, 16, 24, 32, 40, 0, 8, 8, 0, 16, 24, 32, 40, 0};
  constexpr uint8_t attackerOrd[16] = {0, 6, 1, 5, 4, 3, 2, 0, 0, 6, 1, 5, 4, 3, 2, 0};
  for (size_t i = 0; i < count; ++i) {
    Move &move = moves[i];
    move.tag = victimOrd[board.cells[move.dst]] + attackerOrd[board.cells[move.src]];
  }
  std::sort(moves, moves + count, [](const Move m1, const Move m2) { return m1.tag > m2.tag; });
  for (size_t i = 0; i < count; ++i) {
    moves[i].tag = 0;
  }
}

QuiescenseMovePicker::QuiescenseMovePicker(const Board &board)
    : gen_(board), moveCount_(gen_.genCaptures(moves_)) {
  sortMvvLva(board, moves_, moveCount_);
}

void QuiescenseMovePicker::addSimplePromotes() {
  moveCount_ = gen_.genSimplePromotes(moves_);
  movePosition_ = 0;
  std::sort(moves_, moves_ + moveCount_, [](const Move m1, const Move m2) {
    return static_cast<int8_t>(m1.kind) > static_cast<int8_t>(m2.kind);
  });
}

inline static bool isValidKiller(const Board &board, const Move move) {
  return !isMoveCapture(board, move) && !isMoveKindPromote(move.kind) && isMoveValid(board, move);
}

void MovePicker::nextStage() {
  movePosition_ = 0;
  moveCount_ = 0;
  while (moveCount_ == 0) {
    if (stage_ != MovePickerStage::End) {
      stage_ = static_cast<MovePickerStage>(static_cast<int>(stage_) + 1);
    }
    switch (stage_) {
      case MovePickerStage::Start: {
        SOF_UNREACHABLE();
        break;
      }
      case MovePickerStage::HashMove: {
        // Try the move from hash
        if (hashMove_ != Move::null()) {
          moves_[moveCount_++] = hashMove_;
        }
        break;
      }
      case MovePickerStage::Capture: {
        // Generate captures and sort them by MVV/LVA
        moveCount_ = gen_.genCaptures(moves_);
        sortMvvLva(gen_.board(), moves_, moveCount_);
        break;
      }
      case MovePickerStage::SimplePromote: {
        // Generate simple promotes and sort them by promoting piece
        moveCount_ = gen_.genSimplePromotes(moves_);
        std::sort(moves_, moves_ + moveCount_, [](const Move m1, const Move m2) {
          return static_cast<int8_t>(m1.kind) > static_cast<int8_t>(m2.kind);
        });
        break;
      }
      case MovePickerStage::Killer: {
        // Try two killers if they are valid
        const Move firstKiller = killers_.first();
        if (isValidKiller(gen_.board(), firstKiller)) {
          moves_[moveCount_++] = firstKiller;
        }
        const Move secondKiller = killers_.second();
        if (isValidKiller(gen_.board(), secondKiller)) {
          moves_[moveCount_++] = secondKiller;
        }
        savedKillers_[0] = firstKiller;
        savedKillers_[1] = secondKiller;
        break;
      }
      case MovePickerStage::History: {
        // Sort the moves by history heuristic
        moveCount_ = gen_.genSimpleMovesNoPromote(moves_);
        std::sort(moves_, moves_ + moveCount_,
                  [this](const Move m1, const Move m2) { return history_[m1] > history_[m2]; });
        for (size_t i = 0; i < moveCount_; ++i) {
          if (moves_[i] == savedKillers_[0] || moves_[i] == savedKillers_[1]) {
            moves_[i] = Move::null();
          }
        }
        break;
      }
      case MovePickerStage::End: {
        // Invalid move indicates the end of the move list
        moves_[moveCount_++] = Move::invalid();
        break;
      }
    }
  }
}

}  // namespace SoFSearch::Private
