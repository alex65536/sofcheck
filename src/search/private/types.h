// This file is part of SoFCheck
//
// Copyright (c) 2020, 2022 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_SEARCH_PRIVATE_TYPES_INCLUDED
#define SOF_SEARCH_PRIVATE_TYPES_INCLUDED

#include <vector>

#include "core/board.h"
#include "core/move.h"

namespace SoFSearch::Private {

// Position with saved previous moves
struct Position {
  SoFCore::Board first;
  std::vector<SoFCore::Move> moves;
  SoFCore::Board last;

  // Constructs `Position` from `first` and `moves`, calculating `last`
  inline static Position from(const SoFCore::Board &first, std::vector<SoFCore::Move> moves) {
    Position position{first, std::move(moves), first};
    for (SoFCore::Move move : position.moves) {
      moveMake(position.last, move);
    }
    return position;
  }
};

// Special return value for `commonPrefix` function
constexpr size_t COMMON_PREFIX_NONE = static_cast<size_t>(-1);

// Calculates maximum number of common first moves between positions `p1` and `p2`. If first boards
// of positions differ, returns `COMMON_PREFIX_NONE`
size_t commonPrefix(const Position &p1, const Position &p2);

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_TYPES_INCLUDED
