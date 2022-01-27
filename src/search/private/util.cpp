// This file is part of SoFCheck
//
// Copyright (c) 2020-2022 Alexander Kernozhitsky and SoFCheck contributors
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

#include "search/private/util.h"

#include <utility>

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
      board_hash_t &val = newTab[idx + j];
      if (val == 0) {
        val = item;
        break;
      }
    }
  }
  tab_ = std::move(newTab);
  bucketCount_ = newBucketCount;
  mask_ = newMask;
}

}  // namespace SoFSearch::Private
