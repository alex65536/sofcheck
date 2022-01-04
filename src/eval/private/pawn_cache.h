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

namespace SoFEval::Private {

template <typename S>
class PawnCache {
public:
  PawnCache() = default;

  // Try to get the score from the cache if the position of pawns is specified by hash `pawnHash`.
  // If the value is not found, call `func()` to calculate the score and cache the result.
  template <typename F>
  S get(SoFCore::board_hash_t /*pawnHash*/, F func) {
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
  PawnCache();

  template <typename F>
  score_t get(SoFCore::board_hash_t pawnHash, F func) {
    const size_t key = keyFromHash(pawnHash);
    Entry &entry = entries_[key];
    if (entry.hashMatches(pawnHash)) {
      const score_t score = entry.score();
      if (score != SCORE_INVALID) {
        return score;
      }
    }
    const score_t score = func();
    entry = Entry::from(pawnHash, score);
    return score;
  }

private:
  // Must be power of two
  static constexpr size_t CACHE_SIZE = 1U << 16;

  // Only 48 bits of hash are stored in the entries, so we need the size at least 2^16 to use the
  // entire hash
  static_assert(CACHE_SIZE >= (1U << 16));

  static size_t keyFromHash(const SoFCore::board_hash_t hash) { return hash & (CACHE_SIZE - 1); }

  struct Entry {
    uint64_t value;

    static_assert(sizeof(score_t) == 2);
    static_assert(sizeof(SoFCore::board_hash_t) == 8);

    static constexpr Entry from(const uint64_t hash, const score_t score) {
      return {(hash & ~static_cast<uint64_t>(0xffffU)) | static_cast<uint16_t>(score)};
    }

    static constexpr Entry invalid() { return from(0U, SCORE_INVALID); }

    constexpr score_t score() const { return static_cast<score_t>(value & 0xffffU); }

    constexpr bool hashMatches(const uint64_t hash) const {
      return ((hash ^ value) & ~static_cast<uint64_t>(0xffffU)) == 0;
    }
  };

  Entry entries_[CACHE_SIZE];
};

}  // namespace SoFEval::Private

#endif  // SOF_EVAL_PRIVATE_PAWN_CACHE_INCLUDED
