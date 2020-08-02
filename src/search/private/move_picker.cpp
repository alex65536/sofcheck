#include "search/private/move_picker.h"

#include <algorithm>

#include "core/movegen.h"
#include "util/misc.h"

namespace SoFSearch::Private {

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
        // Generate captures and arrange them by MVV/LVA
        moveCount_ = genCaptures(board_, moves_);
        constexpr uint8_t victimOrd[16] = {8, 8, 0, 16, 24, 32, 40, 0, 8, 8, 0, 16, 24, 32, 40, 0};
        constexpr uint8_t attackerOrd[16] = {0, 6, 1, 5, 4, 3, 2, 0, 0, 6, 1, 5, 4, 3, 2, 0};
        for (size_t i = 0; i < moveCount_; ++i) {
          Move &move = moves_[i];
          move.tag = victimOrd[board_.cells[move.dst]] + attackerOrd[board_.cells[move.src]];
        }
        std::sort(moves_, moves_ + moveCount_,
                  [&](const Move m1, const Move m2) { return m1.tag > m2.tag; });
        for (size_t i = 0; i < moveCount_; ++i) {
          moves_[i].tag = 0;
        }
        break;
      }
      case MovePickerStage::Killer: {
        // Try two killers if they are valid
        const Move firstKiller = killers_.first();
        if (isMoveValid(board_, firstKiller)) {
          moves_[moveCount_++] = firstKiller;
        }
        const Move secondKiller = killers_.second();
        if (isMoveValid(board_, secondKiller)) {
          moves_[moveCount_++] = secondKiller;
        }
        savedKillers_[0] = firstKiller;
        savedKillers_[1] = secondKiller;
        break;
      }
      case MovePickerStage::History: {
        // Sort the moves by history heuristic
        moveCount_ = genSimpleMoves(board_, moves_);
        std::sort(moves_, moves_ + moveCount_,
                  [&](const Move m1, const Move m2) { return history_[m1] > history_[m2]; });
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
