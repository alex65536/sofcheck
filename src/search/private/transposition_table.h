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

    inline constexpr score_t score() const { return score_; }
    inline constexpr uint8_t depth() const { return move_.tag; }
    inline constexpr bool isValid() const { return !(flags_ & FLAG_IS_INVALID); }
    inline constexpr bool isPv() const { return flags_ & FLAG_IS_PV; }
    inline constexpr SoFBotApi::PositionCostBound bound() const {
      return static_cast<SoFBotApi::PositionCostBound>(flags_ & 3);
    }

    inline constexpr Data(const SoFCore::Move move, const score_t score, const uint8_t depth,
                          const SoFBotApi::PositionCostBound bound, const bool isPv)
        : move_(move),
          score_(score),
          flags_(static_cast<uint8_t>(bound) | (isPv ? FLAG_IS_PV : 0)),
          epoch_(0) {
      move_.tag = depth;
    }

    inline Data() noexcept = default;

    inline static constexpr Data zero() {
      return Data(PrivateTag{}, SoFCore::Move::null(), 0, 0, 0);
    }

    inline static constexpr Data invalid() {
      return Data(PrivateTag{}, SoFCore::Move::null(), 0, FLAG_IS_INVALID, 0);
    }

    // Serializes the structure as `uint64_t`. Should work efficiently for little-endian
    // architectures (as the compiler must just reinterpret the structure as `uint64_t`).
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

    inline constexpr Data(PrivateTag, const SoFCore::Move move, const score_t score,
                          const uint8_t flags, const uint8_t epoch)
        : move_(move), score_(score), flags_(flags), epoch_(epoch) {}

    SoFCore::Move move_;
    score_t score_;
    uint8_t flags_;
    uint8_t epoch_;

    friend class TranspositionTable;

    // Computes the data weight of the table entry if the current epoch of the transposition table
    // is `curEpoch`. Data weight controls entry replacement, so entries with lower weight are
    // overwritten by entries with greater weight.
    int32_t weight(uint8_t curEpoch) const;

    static constexpr uint8_t FLAG_IS_INVALID = 8;
    static constexpr uint8_t FLAG_IS_PV = 16;
  };

  // Default size of the transposition table
  constexpr static size_t DEFAULT_SIZE = 1 << 25;

  TranspositionTable();

  // Resizes the hash table. The new table size (in bytes) will be the maximum power of two not
  // exceeding `max(1048576, maxSize)`. If `clearTable` is `true`, the table is cleared after
  // resize. Otherwise, we try to retain some information that already exists in the hash table.
  //
  // This function is not thread-safe. No other thread should use the table while resizing.
  void resize(size_t maxSize, bool clearTable);

  // Increments the hash table epoch. It is recommended to call this function once before the new
  // search is started. Note that this function is not thread-safe.
  inline void nextEpoch() { ++epoch_; }

  // Returns the hash table size (in bytes)
  inline size_t sizeBytes() const { return size_ * sizeof(Entry); }

  // Clears the hash table. This function is not thread-safe. No other thread should use the table
  // while resizing.
  void clear();

  // Try to load the entry with the key `key` into CPU cache. You can invoke the method early before
  // you plan to use the cache entry and do soemthing before it loads into CPU cache.
  void prefetch(SoFCore::board_hash_t key);

  // Returns the entry with the key `key`. If such entry doesn't exist, returns `Data::invalid()`.
  Data load(SoFCore::board_hash_t key) const;

  // Stores `value` for the key `key`.
  void store(SoFCore::board_hash_t key, Data value);

private:
  struct Entry {
    std::atomic<Data> value;
    std::atomic<SoFCore::board_hash_t> key;

    inline void clear() { assignRelaxed(Data::zero(), 0); }

    inline void assignRelaxed(const Data value, const SoFCore::board_hash_t key) {
      this->value.store(value, std::memory_order_relaxed);
      this->key.store(key, std::memory_order_relaxed);
    }

    inline void assignRelaxed(const Entry &o) {
      assignRelaxed(o.value.load(std::memory_order_relaxed), o.key.load(std::memory_order_relaxed));
    }
  };

  friend void doClear(Entry *table, size_t size);

  static_assert(std::atomic<SoFCore::board_hash_t>::is_always_lock_free);
  static_assert(std::atomic<Data>::is_always_lock_free);
  static_assert(sizeof(Entry) == 16);

  size_t size_;  // Number of entries, must be power of two
  std::unique_ptr<Entry[]> table_;
  uint8_t epoch_ = 0;
};

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_TRANSPOSITION_TABLE_INCLUDED
