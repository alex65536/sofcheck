#ifndef SOF_SEARCH_PRIVATE_UTIL_INCLUDED
#define SOF_SEARCH_PRIVATE_UTIL_INCLUDED

#include <memory>

#include "core/move.h"
#include "core/types.h"
#include "util/misc.h"

namespace SoFSearch::Private {

// Line of "killer" moves (i.e. the moves that were considered good on a specific depth)
class KillerLine {
public:
  // Returns first killer
  inline constexpr SoFCore::Move first() const { return first_; }

  // Returns second killer
  inline constexpr SoFCore::Move second() const { return second_; }

  // Adds killer move to the line
  inline constexpr void add(const SoFCore::Move move) {
    if (move == first_) {
      return;
    }
    second_ = first_;
    first_ = move;
  }

private:
  SoFCore::Move first_ = SoFCore::Move::null();
  SoFCore::Move second_ = SoFCore::Move::null();
};

// History table used for history heuristics
class HistoryTable {
public:
  HistoryTable() : tab_(std::make_unique<uint64_t[]>(TAB_SIZE)) {}

  inline uint64_t &operator[](const SoFCore::Move move) { return tab_[indexOf(move)]; }
  inline uint64_t operator[](const SoFCore::Move move) const { return tab_[indexOf(move)]; }

private:
  inline constexpr static size_t indexOf(const SoFCore::Move move) {
    return (static_cast<size_t>(move.src) << 6) | static_cast<size_t>(move.dst);
  }

  std::unique_ptr<uint64_t[]> tab_;

  static constexpr size_t TAB_SIZE = 64 * 64;
};

// Small hash table to track draw by repetitions
class RepetitionTable {
public:
  inline RepetitionTable()
      : tab_(std::make_unique<SoFCore::board_hash_t[]>(INITIAL_BUCKET_COUNT * BUCKET_SIZE)),
        bucketCount_(INITIAL_BUCKET_COUNT),
        mask_((INITIAL_BUCKET_COUNT - 1) * BUCKET_SIZE) {}

  // Returns `true` if `board` is present in the hash table
  inline bool has(const SoFCore::board_hash_t board) const {
    const size_t idx = board & mask_;
    for (size_t i = 0; i < BUCKET_SIZE; ++i) {
      if (tab_[idx + i] == board) {
        return true;
      }
    }
    return false;
  }

  // Inserts `board` into the hash table. Returns `false` if `board` is already present
  inline bool insert(const SoFCore::board_hash_t board) {
    if (has(board)) {
      return false;
    }
    for (;;) {
      const size_t idx = board & mask_;
      for (size_t i = 0; i < BUCKET_SIZE; ++i) {
        if (SOF_LIKELY(tab_[idx + i] == 0)) {
          tab_[idx + i] = board;
          return true;
        }
      }
      grow();
    }
  }

  // Removes `board` from the hash table. If it's not present, the behavior is undefined
  inline void erase(const SoFCore::board_hash_t board) {
    const size_t idx = board & mask_;
    for (size_t i = 0; i < BUCKET_SIZE; ++i) {
      uint64_t &val = tab_[idx + i];
      if (val == board) {
        val = 0;
        return;
      }
    }
  }

private:
  void grow();

  std::unique_ptr<SoFCore::board_hash_t[]> tab_;
  size_t bucketCount_;
  size_t mask_;  // Equal to `(bucketCount - 1) * BUCKET_SIZE`

  static constexpr size_t INITIAL_BUCKET_COUNT = 32;
  static constexpr size_t BUCKET_SIZE = 4;
};

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_UTIL_INCLUDED
