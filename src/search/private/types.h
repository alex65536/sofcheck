#ifndef SOF_SEARCH_PRIVATE_TYPES_INCLUDED
#define SOF_SEARCH_PRIVATE_TYPES_INCLUDED

#include <algorithm>
#include <memory>

#include "core/move.h"

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

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_TYPES_INCLUDED
