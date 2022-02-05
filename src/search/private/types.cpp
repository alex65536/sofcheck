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

#include "search/private/types.h"

#include <algorithm>

namespace SoFSearch::Private {

size_t commonPrefix(const Position &p1, const Position &p2) {
  if (p1.first != p2.first) {
    return COMMON_PREFIX_NONE;
  }
  const size_t limit = std::min(p1.moves.size(), p2.moves.size());
  for (size_t idx = 0; idx < limit; ++idx) {
    if (p1.moves[idx] != p2.moves[idx]) {
      return idx;
    }
  }
  return limit;
}

}  // namespace SoFSearch::Private
