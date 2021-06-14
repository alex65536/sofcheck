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

#ifndef SOF_SEARCH_PRIVATE_TRANSPOSITION_TABLE_INCLUDED
#define SOF_SEARCH_PRIVATE_TRANSPOSITION_TABLE_INCLUDED

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "bot_api/types.h"
#include "core/move.h"
#include "core/types.h"
#include "eval/score.h"
#include "util/no_copy_move.h"

namespace SoFSearch::Private {

// Stores the information about the already searched nodes in a hash table
class TranspositionTable : public SoFUtil::NoCopy {
public:
  // Transposition table entry which contains a search result for some position
  class Data {
  public:
    inline constexpr SoFCore::Move move() const {
      SoFCore::Move result = move_;
      result.tag = 0;
      return result;
    }

    inline constexpr SoFEval::score_t score() const { return score_; }
    inline constexpr int32_t depth() const { return static_cast<int32_t>(move_.tag); }
    inline constexpr bool isValid() const { return flags_ & FLAG_IS_VALID; }
    inline constexpr bool isPv() const { return flags_ & FLAG_IS_PV; }
    inline constexpr SoFBotApi::PositionCostBound bound() const {
      return static_cast<SoFBotApi::PositionCostBound>(flags_ & 3);
    }

    inline constexpr Data(const SoFCore::Move move, const SoFEval::score_t score,
                          const int32_t depth, const SoFBotApi::PositionCostBound bound,
                          const bool isPv)
        : move_(move),
          score_(score),
          flags_(static_cast<uint8_t>(bound) | FLAG_IS_VALID | (isPv ? FLAG_IS_PV : 0)),
          epoch_(0) {
      move_.tag = static_cast<uint8_t>(depth);
    }

    inline Data() noexcept = default;

    inline static constexpr Data zero() {
      return Data(PrivateTag{}, SoFCore::Move::null(), 0, 0, 0);
    }

    // Serializes the structure as `uint64_t`. Should work efficiently for little-endian
    // architectures (as the compiler must just reinterpret the structure as `uint64_t`).
    //
    // It seems that such optimization doesn't work with MSVC, so the function may work relatively
    // slow for this compiler.
    inline constexpr uint64_t asUint() const {
      const auto uintScore = static_cast<uint16_t>(score_);
      return static_cast<uint64_t>(move_.asUint()) | (static_cast<uint64_t>(uintScore) << 32) |
             (static_cast<uint64_t>(flags_) << 48) | (static_cast<uint64_t>(epoch_) << 56);
    }

    inline constexpr friend bool operator==(const Data d1, const Data d2) {
      return d1.asUint() == d2.asUint();
    }

    inline constexpr friend bool operator!=(const Data d1, const Data d2) {
      return d1.asUint() == d2.asUint();
    }

  private:
    // Tag to explicitly mark the private constructor
    struct PrivateTag {};

    inline constexpr Data(PrivateTag, const SoFCore::Move move, const SoFEval::score_t score,
                          const uint8_t flags, const uint8_t epoch)
        : move_(move), score_(score), flags_(flags), epoch_(epoch) {}

    SoFCore::Move move_;
    SoFEval::score_t score_;
    uint8_t flags_;
    uint8_t epoch_;

    friend class TranspositionTable;

    // Computes the data weight of the table entry if the current epoch of the transposition table
    // is `curEpoch`. Data weight controls entry replacement, so entries with lower weight are
    // overwritten by entries with greater weight.
    int32_t weight(uint8_t curEpoch) const;

    static constexpr uint8_t FLAG_IS_VALID = 8;
    static constexpr uint8_t FLAG_IS_PV = 16;
  };

  // Default size of the transposition table
  constexpr static size_t DEFAULT_SIZE = 1 << 25;

  TranspositionTable();

  // Resizes the hash table. The new table size (in bytes) will be the maximum power of two not
  // exceeding `max(1048576, maxSize)`. If `clearTable` is `true`, the table is cleared after
  // resize. Otherwise, we try to retain some information that already exists in the hash table.
  // Note that the hash table is resized in a multithreaded way, using `jobs` threads.
  //
  // This function is not thread-safe. No other thread should use the table while resizing.
  void resize(size_t maxSize, bool clearTable, size_t jobs);

  // Increments the hash table epoch. It is recommended to call this function once before the new
  // search is started. Note that this function is not thread-safe.
  inline void nextEpoch() { ++epoch_; }

  // Returns the hash table size (in bytes)
  inline size_t sizeBytes() const { return size_ * sizeof(Entry); }

  // Clears the hash table. The hash table is cleared in a multithreaded way, using `jobs` threads.
  //
  // This function is not thread-safe. No other thread should use the table while resizing.
  void clear(size_t jobs);

  // Try to load the entry with the key `key` into CPU cache. You can invoke the method early before
  // you plan to use the cache entry and do soemthing before it loads into CPU cache.
  void prefetch(SoFCore::board_hash_t key);

  // Returns the entry with the key `key`. If such entry doesn't exist, returns `Data::zero()`.
  Data load(SoFCore::board_hash_t key) const;

  // Stores `value` for the key `key`.
  void store(SoFCore::board_hash_t key, Data value);

  // Stores `value` for the key `key` only if the epoch of `value` differs from the epoch of table
  inline void refresh(const SoFCore::board_hash_t key, const Data value) {
    if (value.epoch_ != epoch_) {
      store(key, value);
    }
  }

private:
  struct Entry {
    std::atomic<Data> value;
    std::atomic<SoFCore::board_hash_t> key;

    inline void clear() { assignRelaxed(Data::zero(), 0); }

    inline void assignRelaxed(const Data newValue, const SoFCore::board_hash_t newKey) {
      this->value.store(newValue, std::memory_order_relaxed);
      this->key.store(newKey, std::memory_order_relaxed);
    }

    inline void assignRelaxed(const Entry &o) {
      assignRelaxed(o.value.load(std::memory_order_relaxed), o.key.load(std::memory_order_relaxed));
    }
  };

  friend void doClear(Entry *table, size_t size, size_t jobs);

  static_assert(std::atomic<SoFCore::board_hash_t>::is_always_lock_free);
  static_assert(std::atomic<Data>::is_always_lock_free);
  static_assert(sizeof(Entry) == 16);

  size_t size_;  // Number of entries, must be power of two
  std::unique_ptr<Entry[]> table_;
  uint8_t epoch_ = 0;
};

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_TRANSPOSITION_TABLE_INCLUDED
