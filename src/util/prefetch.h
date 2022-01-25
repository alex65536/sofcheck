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

#ifndef SOF_UTIL_PREFETCH_INCLUDED
#define SOF_UTIL_PREFETCH_INCLUDED

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace SoFUtil {

// Indicates whether we want to prefetch for reading or for writing
enum class PrefetchKind : int { Read = 0, Write = 1 };

// Prefetch temporal locality. `L0` means no temporal locality, so the value can be removed from the
// cache after being used. `L3` means the highest degree of temporal locality, so the value should
// be left in all the cache levels. `L1` and `L2` mean low and moderate degree of temporal locality,
// respectively
//
// As you can see, locality types are the same as in GCC's `__builtin_prefetch`. For details, see
// https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
//
// Note that there's NO guarantee on numeric values for the members of this enumeration. It's solely
// compiler-dependant
#ifdef _MSC_VER
enum class PrefetchLocality : int {
  L0 = _MM_HINT_NTA,
  L1 = _MM_HINT_T2,
  L2 = _MM_HINT_T1,
  L3 = _MM_HINT_T0
};
#else
enum class PrefetchLocality : int { L0 = 0, L1 = 1, L2 = 2, L3 = 3 };
#endif

// Prefetches the data pointed by address `addr`, to the cache. Template parameters mean prefetch
// type and prefetch locality, respectively (see above for more details)
template <PrefetchKind Kind = PrefetchKind::Read, PrefetchLocality Locality = PrefetchLocality::L3>
void prefetch(const void *addr) {
#ifdef _MSC_VER
  _mm_prefetch(static_cast<char *>(addr), static_cast<int>(Locality));
#else
  __builtin_prefetch(addr, static_cast<int>(Kind), static_cast<int>(Locality));
#endif
}

}  // namespace SoFUtil

#endif  // SOF_UTIL_PREFETCH_INCLUDED
