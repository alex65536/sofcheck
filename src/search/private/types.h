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

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_TYPES_INCLUDED
