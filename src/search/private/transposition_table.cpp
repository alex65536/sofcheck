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

#include "search/private/transposition_table.h"

#include <algorithm>
#include <limits>
#include <utility>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include "util/parallel.h"

namespace SoFSearch::Private {

using SoFCore::board_hash_t;

void doClear(TranspositionTable::Entry *table, const size_t size, const size_t jobs) {
  SoFUtil::processSegmentParallel(0, size, jobs, [table](const size_t left, const size_t right) {
    for (size_t i = left; i < right; ++i) {
      table[i].clear();
    }
  });
}

int32_t TranspositionTable::Data::weight(const uint8_t curEpoch) const {
  if (!isValid()) {
    return std::numeric_limits<int32_t>::min();
  }
  const uint8_t age = curEpoch - epoch_;
  int32_t result = 4 * depth() - age;
  if (bound() == SoFBotApi::PositionCostBound::Exact) {
    result += 6;
  }
  if (move() == SoFCore::Move::null()) {
    result -= 4;
  }
  if (isPv()) {
    result += 2;
  }
  return result;
}

void TranspositionTable::clear(const size_t jobs) { doClear(table_.get(), size_, jobs); }

TranspositionTable::Data TranspositionTable::load(const board_hash_t key) const {
  const size_t idx = key & (size_ - 1);
  const Entry &entry = table_[idx];
  const Data entryData = entry.value.load(std::memory_order_relaxed);
  const board_hash_t entryKey = entry.key.load(std::memory_order_relaxed) ^ entryData.asUint();
  if (entryKey != key) {
    return Data::zero();
  }
  return entryData;
}

void TranspositionTable::prefetch(const board_hash_t key) {
  const size_t idx = key & (size_ - 1);
#ifdef _MSC_VER
  _mm_prefetch(reinterpret_cast<char *>(&table_[idx]), 0);
#else
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
  __builtin_prefetch(&table_[idx], 0, 1);
#endif
}

void TranspositionTable::resize(size_t maxSize, const bool clearTable, const size_t jobs) {
  maxSize = std::max<size_t>(maxSize, 1 << 20);

  // Determine the new table size
  size_t newSize = 1;
  while (newSize <= maxSize) {
    newSize <<= 1;
  }
  newSize >>= 1;
  newSize /= sizeof(Entry);
  if (newSize == size_) {
    if (clearTable) {
      clear(jobs);
    }
    return;
  }

  // Do not use `std::make_unique` here, as we want the array to be uninitialized
  std::unique_ptr<Entry[]> newData(new Entry[newSize]);
  if (clearTable) {
    doClear(newData.get(), newSize, jobs);
  } else if (newSize > size_) {
    doClear(newData.get(), newSize, jobs);
    SoFUtil::processSegmentParallel(
        0, size_, jobs, [this, newSize, &newData](const size_t left, const size_t right) {
          for (size_t i = left; i < right; ++i) {
            const Entry &entry = table_[i];
            const Data value = entry.value.load(std::memory_order_relaxed);
            const board_hash_t key = entry.key.load(std::memory_order_relaxed);
            const size_t idx = (key ^ value.asUint()) & (newSize - 1);
            newData[idx].assignRelaxed(value, key);
          }
        });
  } else {
    SoFUtil::processSegmentParallel(
        0, newSize, jobs, [this, newSize, &newData](const size_t left, const size_t right) {
          for (size_t i = left; i < right; ++i) {
            newData[i].assignRelaxed(table_[i]);
          }
          const uint8_t epoch = epoch_;
          for (size_t offset = newSize; offset < size_; offset += newSize) {
            for (size_t i = left; i < right; ++i) {
              const Entry &oldEntry = table_[i + offset];
              Entry &newEntry = newData[i];
              if (oldEntry.value.load(std::memory_order_relaxed).weight(epoch) >
                  newEntry.value.load(std::memory_order_relaxed).weight(epoch)) {
                newEntry.assignRelaxed(oldEntry);
              }
            }
          }
        });
  }

  table_ = std::move(newData);
  size_ = newSize;
}

void TranspositionTable::store(board_hash_t key, TranspositionTable::Data value) {
  const size_t idx = key & (size_ - 1);
  const uint8_t epoch = epoch_;
  Entry &entry = table_[idx];
  value.epoch_ = epoch;
  if (entry.value.load(std::memory_order_relaxed).weight(epoch) > value.weight(epoch)) {
    return;
  }
  key ^= value.asUint();
  entry.assignRelaxed(value, key);
}

TranspositionTable::TranspositionTable()
    : size_(DEFAULT_SIZE / sizeof(Entry)), table_(new Entry[size_]) {
  clear(1);
}

}  // namespace SoFSearch::Private
