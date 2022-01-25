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

#ifndef SOF_EVAL_PRIVATE_CACHE_INCLUDED
#define SOF_EVAL_PRIVATE_CACHE_INCLUDED

#include <algorithm>
#include <cstdint>
#include <type_traits>

#include "core/types.h"
#include "eval/score.h"
#include "util/misc.h"

namespace SoFEval::Private {

// Hash type
using hash_t = uint64_t;
static_assert(std::is_same_v<hash_t, SoFCore::board_hash_t>);

// Base class for evaluation cache. It is used to create a cache with score type `S`, value type
// `Value` and size `2 ^ SizePow`. Note that the cache is a no-op for all score types except
// `score_t`
template <typename S, typename Value, size_t SizePow>
class Cache {
public:
  static constexpr size_t SIZE = 0;

  Cache() = default;

  // Try to get the value from the cache by key `hash`. If the value is not found, call `func()` to
  // calculate and cache the result
  template <typename F>
  Value get(hash_t /*key*/, F func) {
    // No value is cached in this implementation, so just run `func` to get the value
    return func();
  }
};

template <typename Value, size_t SizePow>
class Cache<score_t, Value, SizePow> {
public:
  static constexpr size_t SIZE = static_cast<size_t>(1) << SizePow;

  Cache() { std::fill(entries_, entries_ + SIZE, Entry::invalid()); }

  template <typename F>
  Value get(const hash_t key, F func) {
    const size_t index = indexFromKey(key);
    Entry &entry = entries_[index];
    if (SOF_LIKELY(entry.key == key)) {
      const Value &value = entry.value;
      if (SOF_LIKELY(value.isValid())) {
        return value;
      }
    }
    const Value value = func();
    entry = Entry::from(key, value);
    return value;
  }

private:
  static size_t indexFromKey(const hash_t key) { return key & (SIZE - 1); }

  struct Entry {
    hash_t key;
    Value value;

    static constexpr Entry from(const hash_t key, Value value) { return {key, std::move(value)}; }
    static constexpr Entry invalid() { return from(0U, Value::invalid()); }
  };

  Entry entries_[SIZE];
};

// Value for `PawnCache`
template <typename S>
struct PawnCacheValue {
  static constexpr uint8_t FLAG_IS_VALID = 1;

  constexpr bool isValid() const { return flags & FLAG_IS_VALID; }

  // Creates a `PawnCacheValue` that corresponds to an invalid cache entry
  static constexpr PawnCacheValue invalid() { return {0, 0, 0, 0, 0, S{}}; }

  // Creates a `PawnCacheValue` that correspond to a valid cache entry
  static constexpr PawnCacheValue from(const uint8_t bbOpenCols, const uint8_t bbWhiteOnlyCols,
                                       const uint8_t bbBlackOnlyCols, S score) {
    return {bbOpenCols, bbWhiteOnlyCols, bbBlackOnlyCols, FLAG_IS_VALID, 0, std::move(score)};
  }

  uint8_t bbOpenCols;
  uint8_t bbWhiteOnlyCols;
  uint8_t bbBlackOnlyCols;
  uint8_t flags;
  uint16_t unused;  // Required for alignment, must be set to zero
  S score;
};

static_assert(sizeof(PawnCacheValue<score_t>) == 8);

// Pawn cache. In this cache, `key` must be the hash of pawn positions (both white and black)
template <typename S>
class PawnCache : public Cache<S, PawnCacheValue<S>, 18> {
public:
  using Value = PawnCacheValue<S>;
  using Cache<S, Value, 18>::get;
  using Cache<S, Value, 18>::SIZE;
};

}  // namespace SoFEval::Private

#endif  // SOF_EVAL_PRIVATE_CACHE_INCLUDED
