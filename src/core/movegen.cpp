#include "movegen.h"

#include "core/private/near_attacks.h"

namespace SoFCore {

template <Color C>
bool isCellAttacked(const SoFCore::Board &b, SoFCore::coord_t coord) {
  const cell_t offset = colorOffset(C);
  // Here, we use black attack map for white, as we need to trace the attack from destination piece,
  // not from the source one
  const auto *pawnAttacks =
      (C == Color::White) ? Private::BLACK_PAWN_ATTACKS : Private::WHITE_PAWN_ATTACKS;
  if ((b.bbPieces[offset] & pawnAttacks[coord]) ||
      (b.bbPieces[offset + 1] & Private::KING_ATTACKS[coord]) ||
      (b.bbPieces[offset + 2] & Private::KNIGHT_ATTACKS[coord])) {
    return true;
  }
  return false;
}

template bool isCellAttacked<Color::White>(const Board &b, coord_t coord);
template bool isCellAttacked<Color::Black>(const Board &b, coord_t coord);

}  // namespace SoFCore
