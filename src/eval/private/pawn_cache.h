// This file is part of SoFCheck
//
// Copyright (c) 2022 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_EVAL_PRIVATE_PAWN_CACHE_INCLUDED
#define SOF_EVAL_PRIVATE_PAWN_CACHE_INCLUDED

#include "core/board.h"
#include "core/move.h"
#include "core/types.h"
#include "eval/score.h"
#include "util/misc.h"

namespace SoFEval::Private {

// Value stored in the pawn cache
template <typename S>
struct PawnCacheValue {
  static constexpr uint8_t FLAG_IS_VALID = 1;

  constexpr bool isValid() const { return flags & FLAG_IS_VALID; }

  // Creates a `PawnCacheValue` that correspond to an invalid cache entry
  static constexpr PawnCacheValue invalid() { return {0, 0, 0, 0, 0, S{}}; }

  // Creates a `PawnCacheValue` that correspond to a valid cache entry
  static constexpr PawnCacheValue from(const uint8_t bbOpenLine, const uint8_t bbWhiteOnlyLine,
                                       const uint8_t bbBlackOnlyLine, S score) {
    return {bbOpenLine, bbWhiteOnlyLine, bbBlackOnlyLine, FLAG_IS_VALID, 0, std::move(score)};
  }

  uint8_t bbOpenCols;
  uint8_t bbWhiteOnlyCols;
  uint8_t bbBlackOnlyCols;
  uint8_t flags;
  uint16_t unused;  // Required for alignment, must be set to zero
  S score;
};

// Caches evaluation result for pawns
template <typename S>
class PawnCache {
public:
  using Value = PawnCacheValue<S>;

  PawnCache() = default;

  // Try to get the score from the cache if the position of pawns (both white and black) is
  // specified by hash `pawnHash`. If the value is not found, call `func()` to calculate the score
  // and cache the result.
  template <typename F>
  Value get(SoFCore::board_hash_t /*pawnHash*/, F func) {
    // No value is cached in this implementation, so just run `func` to get the score
    return func();
  }
};

// Pawn cache is helpful only for real scores, not for vector ones. So, only the specialization of
// `PawnCache` for `score_t` does something useful.
template <>
class PawnCache<score_t> {
private:
  static constexpr score_t SCORE_INVALID = SCORE_INF;

public:
  using Value = PawnCacheValue<score_t>;

  PawnCache();

  template <typename F>
  Value get(SoFCore::board_hash_t pawnHash, F func) {
    const size_t key = keyFromHash(pawnHash);
    Entry &entry = entries_[key];
    if (SOF_LIKELY(entry.hash == pawnHash)) {
      const Value &value = entry.value;
      if (SOF_LIKELY(value.isValid())) {
        return value;
      }
    }
    const Value value = func();
    entry = Entry::from(pawnHash, value);
    return value;
  }

private:
  // Must be power of two
  static constexpr size_t CACHE_SIZE = 1U << 18;

  // Only 48 bits of hash are stored in the entries, so we need the size at least 2^16 to use the
  // entire hash
  static_assert(CACHE_SIZE >= (1U << 16));

  static size_t keyFromHash(const SoFCore::board_hash_t hash) { return hash & (CACHE_SIZE - 1); }

  struct Entry {
    SoFCore::board_hash_t hash;
    Value value;

    static_assert(sizeof(score_t) == 2);
    static_assert(sizeof(SoFCore::board_hash_t) == 8);
    static_assert(sizeof(Value) == 8);

    static constexpr Entry from(const uint64_t hash, const Value value) { return {hash, value}; }
    static constexpr Entry invalid() { return from(0U, Value::invalid()); }
  };

  Entry entries_[CACHE_SIZE];
};

}  // namespace SoFEval::Private

#endif  // SOF_EVAL_PRIVATE_PAWN_CACHE_INCLUDED
