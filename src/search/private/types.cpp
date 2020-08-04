#include "search/private/types.h"

namespace SoFSearch::Private {

using SoFCore::board_hash_t;

void RepetitionTable::grow() {
  const size_t oldCap = capacity_;
  capacity_ *= 2;
  mask_ = mask_ * 2 + 1;
  --bits_;
  board_hash_t *newTab = new board_hash_t[2 * capacity_];
  for (size_t i = 0; i < 2 * capacity_; ++i) {
    newTab[i] = 0;
  }
  for (size_t i = 0; i < 2 * oldCap; ++i) {
    const board_hash_t item = tab_[i];
    const size_t lo = hashLo(item);
    const size_t idx = (newTab[lo] == 0) ? lo : hashHi(item);
    newTab[idx] = item;
  }
  tab_.reset(newTab);
}

}  // namespace SoFSearch::Private
