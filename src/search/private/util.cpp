#include "search/private/util.h"

namespace SoFSearch::Private {

using SoFCore::board_hash_t;

void RepetitionTable::grow() {
  const size_t newBucketCount = bucketCount_ * 2;
  const size_t newMask = (newBucketCount - 1) * BUCKET_SIZE;
  auto newTab = std::make_unique<board_hash_t[]>(newBucketCount * BUCKET_SIZE);
  for (size_t i = 0; i < bucketCount_ * BUCKET_SIZE; ++i) {
    const board_hash_t item = tab_[i];
    if (item == 0) {
      continue;
    }
    const size_t idx = item & newMask;
    for (size_t j = 0; j < BUCKET_SIZE; ++j) {
      if (newTab[idx + j] == 0) {
        newTab[idx + j] = item;
        break;
      }
    }
  }
  tab_ = std::move(newTab);
  bucketCount_ = newBucketCount;
  mask_ = newMask;
}

}  // namespace SoFSearch::Private
