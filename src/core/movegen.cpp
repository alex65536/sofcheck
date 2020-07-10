#include "movegen.h"

#include "core/private/magic.h"
#include "core/private/near_attacks.h"

namespace SoFCore {

template <Color C>
bool isCellAttacked(const SoFCore::Board &b, SoFCore::coord_t coord) {
  // Here, we use black attack map for white, as we need to trace the attack from destination piece,
  // not from the source one
  const auto *pawnAttacks =
      (C == Color::White) ? Private::BLACK_PAWN_ATTACKS : Private::WHITE_PAWN_ATTACKS;

  // Check near attacks
  if ((b.bbPieces[makeCell(C, Piece::Pawn)] & pawnAttacks[coord]) ||
      (b.bbPieces[makeCell(C, Piece::King)] & Private::KING_ATTACKS[coord]) ||
      (b.bbPieces[makeCell(C, Piece::Knight)] & Private::KNIGHT_ATTACKS[coord])) {
    return true;
  }

  // Check far attacks
  const bitboard_t diagPieces =
      b.bbPieces[makeCell(C, Piece::Bishop)] | b.bbPieces[makeCell(C, Piece::Queen)];
  const bitboard_t linePieces =
      b.bbPieces[makeCell(C, Piece::Rook)] | b.bbPieces[makeCell(C, Piece::Queen)];
  if ((Private::bishopAttackBitboard(b.bbAll, coord) & diagPieces) ||
      (Private::rookAttackBitboard(b.bbAll, coord) & linePieces)) {
    return true;
  }

  return false;
}

template bool isCellAttacked<Color::White>(const Board &b, coord_t coord);
template bool isCellAttacked<Color::Black>(const Board &b, coord_t coord);

}  // namespace SoFCore
