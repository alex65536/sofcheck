#ifndef SOF_SEARCH_PRIVATE_TYPES_INCLUDED
#define SOF_SEARCH_PRIVATE_TYPES_INCLUDED

#include <algorithm>
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
  HistoryTable() : tab_(new uint64_t[TAB_SIZE]) { std::fill(tab_.get(), tab_.get() + TAB_SIZE, 0); }

  inline uint64_t &operator[](const SoFCore::Move move) { return tab_[indexOf(move)]; }
  inline uint64_t operator[](const SoFCore::Move move) const { return tab_[indexOf(move)]; }

private:
  inline constexpr size_t indexOf(const SoFCore::Move move) const {
    return (static_cast<size_t>(move.src) << 6) | static_cast<size_t>(move.dst);
  }

  std::unique_ptr<uint64_t[]> tab_;

  static constexpr size_t TAB_SIZE = 64 * 64;
};

// Small hash table to track draw by repetitions
class RepetitionTable {
public:
  inline RepetitionTable()
      : tab_(new SoFCore::board_hash_t[2 * INITIAL_CAPACITY]),
        capacity_(INITIAL_CAPACITY),
        mask_(INITIAL_CAPACITY - 1),
        bits_(64 - INITIAL_CAPACITY_BITS) {
    std::fill(tab_.get(), tab_.get() + 2 * capacity_, 0);
  }

  // Returns `true` if `board` is present in the hash table
  inline bool has(const SoFCore::board_hash_t board) const {
    return tab_[hashLo(board)] == board || tab_[hashHi(board)] == board;
  }

  // Inserts item `board` into the hash table. Returns `false` if the item is already present
  inline bool insert(const SoFCore::board_hash_t board) {
    if (has(board)) {
      return false;
    }
    for (;;) {
      const size_t lo = hashLo(board);
      if (SOF_LIKELY(tab_[lo] == 0)) {
        tab_[lo] = board;
        return true;
      }
      const size_t hi = hashHi(board);
      if (tab_[hi] == 0) {
        tab_[hi] = board;
        return true;
      }
      grow();
    }
  }

  // Removes `board` from the hash table. If it's not present, the behavior is undefined
  inline void erase(const SoFCore::board_hash_t board) {
    const size_t lo = hashLo(board);
    const size_t idx = (tab_[lo] == board) ? lo : hashHi(board);
    tab_[idx] = 0;
  }

private:
  void grow();

  inline size_t hashLo(const SoFCore::board_hash_t board) const { return board & mask_; }

  inline size_t hashHi(const SoFCore::board_hash_t board) const {
    return capacity_ + (board >> bits_);
  }

  std::unique_ptr<SoFCore::board_hash_t[]> tab_;
  size_t capacity_;
  size_t mask_;  // Equal to `capacity - 1`
  size_t bits_;  // Equal to `64 - log2(capacity)`

  static constexpr size_t INITIAL_CAPACITY_BITS = 6;
  static constexpr size_t INITIAL_CAPACITY = 1 << INITIAL_CAPACITY_BITS;
};

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_TYPES_INCLUDED
