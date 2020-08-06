#ifndef SOF_SEARCH_PRIVATE_TRANSPOSITION_TABLE_INCLUDED
#define SOF_SEARCH_PRIVATE_TRANSPOSITION_TABLE_INCLUDED

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "bot_api/types.h"
#include "core/move.h"
#include "core/types.h"
#include "search/private/score.h"
#include "util/no_copy_move.h"

namespace SoFSearch::Private {

using SoFBotApi::PositionCostBound;
using SoFCore::board_hash_t;
using SoFCore::Move;

class TranspositionTable : public SoFUtil::NoCopy {
public:
  class Data {
  public:
    inline constexpr Move move() const {
      Move result = move_;
      result.tag = 0;
      return result;
    }

    inline constexpr score_t score() const { return score_; }
    inline constexpr uint8_t depth() const { return move_.tag; }
    inline constexpr bool isValid() const { return !(flags_ & FLAG_IS_INVALID); }
    inline constexpr bool isPv() const { return flags_ & FLAG_IS_PV; }
    inline constexpr PositionCostBound bound() const {
      return static_cast<PositionCostBound>(flags_ & 3);
    }

    inline constexpr Data(const Move move, const score_t score, const uint8_t depth,
                          const PositionCostBound bound, const bool isPv)
        : move_(move),
          score_(score),
          flags_(static_cast<uint8_t>(bound) | (isPv ? FLAG_IS_PV : 0)),
          padding_(0) {
      move_.tag = depth;
    }

    inline Data() noexcept {}

    inline static constexpr Data zero() { return Data(PrivateTag{}, Move::null(), 0, 0, 0); }
    inline static constexpr Data invalid() {
      return Data(PrivateTag{}, Move::null(), 0, FLAG_IS_INVALID, 0);
    }

    // Serializes the structure as `uint64_t`. Should work efficiently for little-endian
    // architectures (as the compiler must just reinterpret the structure as `uint64_t`).
    inline constexpr uint64_t asUint() const {
      const auto uintScore = static_cast<uint16_t>(score_);
      return static_cast<uint64_t>(move_.asUint()) | (static_cast<uint64_t>(uintScore) << 32) |
             (static_cast<uint64_t>(flags_) << 48) | (static_cast<uint64_t>(padding_) << 56);
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

    inline constexpr Data(PrivateTag, const Move move, const score_t score, const uint8_t flags,
                          const uint8_t padding)
        : move_(move), score_(score), flags_(flags), padding_(padding) {}

    Move move_;
    score_t score_;
    uint8_t flags_;
    uint8_t padding_;

    static constexpr uint8_t FLAG_IS_INVALID = 8;
    static constexpr uint8_t FLAG_IS_PV = 16;
  };

  // Default size of the transposition table
  constexpr static size_t DEFAULT_SIZE = 1 << 25;

  TranspositionTable();

  // Resize the hash table. The new table size (in bytes) will be the maximum power of two not
  // exceeding `max(1048576, maxSize)`. If `clearTable` is `true`, the table is cleared after
  // resize. Otherwise, we try to retain some information that already exists in the hash table.
  //
  // This function is not thread-safe. No other thread should use the table while resizing.
  void resize(size_t maxSize, bool clearTable);

  // Return the has table size in bytes
  inline size_t sizeBytes() const { return size_ * sizeof(Entry); }

  // This function is not thread-safe. No other thread should use the table while resizing.
  void clear();

  // Try to load the entry with the key `key` into CPU cache. You can invoke the method early before
  // you plan to use the cache entry and do soemthing before it loads into CPU cache.
  void prefetch(board_hash_t key);

  // Returns the entry with the key `key`. If such entry doesn't exist, return `Data::invalid()`.
  Data load(board_hash_t key) const;

  // Stores `value` for the key `key`.
  void store(board_hash_t key, Data value);

private:
  struct Entry {
    std::atomic<Data> value;
    std::atomic<board_hash_t> key;

    inline void clear() { assignRelaxed(Data::zero(), 0); }

    inline void assignRelaxed(const Data value, const board_hash_t key) {
      this->value.store(value, std::memory_order_relaxed);
      this->key.store(key, std::memory_order_relaxed);
    }

    inline void assignRelaxed(const Entry &o) {
      assignRelaxed(o.value.load(std::memory_order_relaxed), o.key.load(std::memory_order_relaxed));
    }
  };

  static void clear(Entry *table, size_t size);

  // Ensure that the hash entry is lock-free
  static_assert(std::atomic<board_hash_t>::is_always_lock_free);
  static_assert(std::atomic<Data>::is_always_lock_free);

  // Ensure that the hash entry size is 16
  static_assert(sizeof(Entry) == 16);

  size_t size_;  // Must be power of two
  std::unique_ptr<Entry[]> table_;
};

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_TRANSPOSITION_TABLE_INCLUDED
