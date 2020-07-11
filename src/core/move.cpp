#include "move.h"

#include <cstdlib>

namespace SoFCore {

bool Move::isWellFormed(Color c) const {
  if (src >= 64 || dst >= 64) {
    return false;
  }
  if (kind != MoveKind::Null && src == dst) {
    return false;
  }
  switch (kind) {
    case MoveKind::Null: {
      if (src != 0 || dst != 0 || promote != 0) {
        return false;
      }
      break;
    }
    case MoveKind::Simple: {
      if (promote != 0) {
        return false;
      }
      break;
    }
    case MoveKind::PawnDoubleMove: {
      if (promote != 0) {
        return false;
      }
      const subcoord_t srcX = (c == Color::White) ? 6 : 1;
      const subcoord_t dstX = (c == Color::White) ? 4 : 3;
      if (coordX(src) != srcX || coordX(dst) != dstX || coordY(src) != coordY(dst)) {
        return false;
      }
      break;
    }
    case MoveKind::Enpassant: {
      if (promote != 0) {
        return false;
      }
      const subcoord_t srcX = (c == Color::White) ? 3 : 4;
      const subcoord_t dstX = (c == Color::White) ? 2 : 5;
      if (coordX(src) != srcX || coordX(dst) != dstX) {
        return false;
      }
      const subcoord_t srcY = coordY(src);
      const subcoord_t dstY = coordY(dst);
      if (srcY + 1 != dstY && srcY - 1 != dstY) {
        return false;
      }
      break;
    }
    case MoveKind::CastlingKingside: {
      if (promote != 0) {
        return false;
      }
      const subcoord_t x = c == Color::White ? 7 : 0;
      if (src != makeCoord(x, 4) || dst != makeCoord(x, 7)) {
        return false;
      }
      break;
    }
    case MoveKind::CastlingQueenside: {
      if (promote != 0) {
        return false;
      }
      const subcoord_t x = c == Color::White ? 7 : 0;
      if (src != makeCoord(x, 4) || dst != makeCoord(x, 0)) {
        return false;
      }
      break;
    }
    case MoveKind::Promote: {
      const subcoord_t srcX = (c == Color::White) ? 1 : 6;
      const subcoord_t dstX = (c == Color::White) ? 0 : 7;
      if (coordX(src) != srcX || coordX(dst) != dstX) {
        return false;
      }
      const subcoord_t srcY = coordY(src);
      const subcoord_t dstY = coordY(dst);
      if (srcY + 1 != dstY && srcY != dstY && srcY - 1 != dstY) {
        return false;
      }
      if (promote != makeCell(c, Piece::Knight) && promote != makeCell(c, Piece::Bishop) &&
          promote != makeCell(c, Piece::Rook) && promote != makeCell(c, Piece::Queen)) {
        return false;
      }
      break;
    }
    default: {
      return false;
    }
  };
  return true;
}

}  // namespace SoFCore
