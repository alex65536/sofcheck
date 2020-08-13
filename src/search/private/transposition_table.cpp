#include "search/private/transposition_table.h"

#include <algorithm>

#include "util/bit.h"

namespace SoFSearch::Private {

// TODO : clear and rehash the transposition table in a multithreaded way

void doClear(TranspositionTable::Entry *table, const size_t size) {
  for (size_t i = 0; i < size; ++i) {
    table[i].clear();
  }
}

void TranspositionTable::clear() { doClear(table_.get(), size_); }

TranspositionTable::Data TranspositionTable::load(const board_hash_t key) const {
  const size_t idx = key & (size_ - 1);
  const Entry &entry = table_[idx];
  const Data entryData = entry.value.load(std::memory_order_relaxed);
  const board_hash_t entryKey = entry.key.load(std::memory_order_relaxed) ^ entryData.asUint();
  if (entryKey != key) {
    return Data::invalid();
  }
  return entryData;
}

void TranspositionTable::prefetch(const board_hash_t key) {
  const size_t idx = key & (size_ - 1);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
  __builtin_prefetch(&table_[idx], 0, 1);
}

void TranspositionTable::resize(size_t maxSize, const bool clearTable) {
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
      clear();
    }
    return;
  }

  std::unique_ptr<Entry[]> newData(new Entry[newSize]);
  if (clearTable) {
    doClear(newData.get(), newSize);
  } else if (newSize > size_) {
    doClear(newData.get(), newSize);
    for (size_t i = 0; i < size_; ++i) {
      const Entry &entry = table_[i];
      const Data value = entry.value.load(std::memory_order_relaxed);
      const board_hash_t key = entry.key.load(std::memory_order_relaxed);
      const size_t idx = (key ^ value.asUint()) & (newSize - 1);
      newData[idx].assignRelaxed(value, key);
    }
  } else {
    // TODO : retain the best entry, not the first one
    for (size_t i = 0; i < newSize; ++i) {
      newData[i].assignRelaxed(table_[i]);
    }
  }

  table_ = std::move(newData);
  size_ = newSize;
}

void TranspositionTable::store(board_hash_t key, const TranspositionTable::Data value) {
  const size_t idx = key & (size_ - 1);
  key ^= value.asUint();
  table_[idx].assignRelaxed(value, key);
}

TranspositionTable::TranspositionTable()
    : size_(DEFAULT_SIZE / sizeof(Entry)), table_(new Entry[size_]) {
  clear();
}

}  // namespace SoFSearch::Private
