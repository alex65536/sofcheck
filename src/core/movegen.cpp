#include "movegen.h"

#include "core/private/bit_consts.h"
#include "core/private/magic.h"
#include "core/private/near_attacks.h"
#include "core/private/part_lines.h"
#include "util/bit.h"
#include "util/compiler.h"

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

template <Color C>
inline static bool isMoveLegalImpl(const Board &b) {
  return isCellAttacked<C>(b, b.kingPos(invert(C)));
}

bool isMoveLegal(const Board &b) {
  return (b.side == Color::White) ? isMoveLegalImpl<Color::White>(b)
                                  : isMoveLegalImpl<Color::Black>(b);
}

template <Color C>
inline static size_t addPawnWithPromote(Move *list, size_t size, const coord_t src,
                                        const coord_t dst, const subcoord_t x) {
  constexpr subcoord_t pawnPromoteLine = (C == Color::White) ? 1 : 6;
  if (x == pawnPromoteLine) {
    list[size++] = Move{MoveKind::Promote, src, dst, makeCell(C, Piece::Knight)};
    list[size++] = Move{MoveKind::Promote, src, dst, makeCell(C, Piece::Bishop)};
    list[size++] = Move{MoveKind::Promote, src, dst, makeCell(C, Piece::Rook)};
    list[size++] = Move{MoveKind::Promote, src, dst, makeCell(C, Piece::Queen)};
  } else {
    list[size++] = Move{MoveKind::Simple, src, dst, 0};
  }
  return size;
}

template <Color C>
inline static size_t genPawnSimple(const Board &b, Move *list) {
  size_t size = 0;
  bitboard_t bbPawns = b.bbPieces[makeCell(C, Piece::Pawn)];
  constexpr subcoord_t pawnDoubleLine = (C == Color::White) ? 6 : 1;
  while (bbPawns) {
    const coord_t src = SoFUtil::extractLowest(bbPawns);
    constexpr coord_t delta = (C == Color::White) ? -8 : 8;
    const coord_t dst = src + delta;
    // We assume that pawns cannot stay on lines 0 and 7, so don't check it
    if (b.cells[dst] != EMPTY_CELL) {
      continue;
    }
    const coord_t x = coordX(src);
    size = addPawnWithPromote<C>(list, size, src, dst, x);
    if (x == pawnDoubleLine) {
      const coord_t dst2 = dst + delta;
      if (b.cells[dst2] == EMPTY_CELL) {
        list[size++] = Move{MoveKind::PawnDoubleMove, src, dst2, 0};
      }
    }
  }
  return size;
}

template <Color C>
inline static size_t genPawnCapture(const Board &b, Move *list) {
  size_t size = 0;
  bitboard_t bbPawns = b.bbPieces[makeCell(C, Piece::Pawn)];
  while (bbPawns) {
    const coord_t src = SoFUtil::extractLowest(bbPawns);
    const subcoord_t x = coordX(src);
    const subcoord_t y = coordY(src);
    // We assume that pawns cannot stay on lines 0 and 7, so don't check it
    constexpr coord_t leftDelta = (C == Color::White) ? -9 : 7;
    constexpr coord_t rightDelta = (C == Color::White) ? -7 : 9;
    const coord_t leftDst = src + leftDelta;
    const coord_t rightDst = src + rightDelta;
    if (y != 0 && cellPieceHasColor(b.cells[leftDst], invert(C))) {
      size = addPawnWithPromote<C>(list, size, src, leftDst, x);
    }
    if (y != 7 && cellPieceHasColor(b.cells[rightDst], invert(C))) {
      size = addPawnWithPromote<C>(list, size, src, rightDst, x);
    }
  }
  return size;
}

template <Color C>
inline static size_t genPawnEnpassant(const Board &b, Move *list) {
  const coord_t enpassantCoord = b.enpassantCoord;
  if (enpassantCoord == INVALID_COORD) {
    return 0;
  }
  size_t size = 0;
  constexpr coord_t delta = (C == Color::White) ? -8 : 8;
  const coord_t y = coordY(enpassantCoord);
  const coord_t dst = enpassantCoord + delta;
  // We assume that the cell behind the pawn that made double move is clean, so don't check it
  const coord_t leftPawn = enpassantCoord - 1;
  if (y != 0 && b.cells[leftPawn] == makeCell(C, Piece::Pawn)) {
    list[size++] = Move{MoveKind::Enpassant, leftPawn, dst, 0};
  }
  const coord_t rightPawn = enpassantCoord + 1;
  if (y != 7 && b.cells[rightPawn] == makeCell(C, Piece::Pawn)) {
    list[size++] = Move{MoveKind::Enpassant, rightPawn, dst, 0};
  }
  return size;
}

template <Color C, Piece P, bool GenSimple, bool GenCaptures>
inline static size_t genKnightOrKing(const Board &b, Move *list, const bitboard_t attacks[]) {
  static_assert(GenSimple || GenCaptures);
  size_t size = 0;
  // Determine what cells we can enter based on what we must generate
  const bitboard_t allowedCells =
      GenSimple ? (GenCaptures ? (BITBOARD_FULL ^ b.bbColor(C)) : (BITBOARD_FULL ^ b.bbAll))
                : b.bbColor(invert(C));
  bitboard_t bbSrc = b.bbPieces[makeCell(C, P)];
  while (bbSrc) {
    const coord_t src = SoFUtil::extractLowest(bbSrc);
    bitboard_t bbDest = attacks[src] & allowedCells;
    while (bbDest) {
      const coord_t dst = SoFUtil::extractLowest(bbDest);
      list[size++] = Move{MoveKind::Simple, src, dst, 0};
    }
  }
  return size;
}

template <Color C, bool GenSimple, bool GenCaptures>
inline static size_t genKnight(const Board &b, Move *list) {
  return genKnightOrKing<C, Piece::Knight, GenSimple, GenCaptures>(b, list,
                                                                   Private::KNIGHT_ATTACKS);
}

template <Color C, bool GenSimple, bool GenCaptures>
inline static size_t genKing(const Board &b, Move *list) {
  return genKnightOrKing<C, Piece::King, GenSimple, GenCaptures>(b, list, Private::KING_ATTACKS);
}

template <Color C>
inline static size_t genBishopCapture(const Board &b, Move *list, bitboard_t bbSrc) {
  size_t size = 0;
  while (bbSrc) {
    const coord_t src = SoFUtil::extractLowest(bbSrc);
    bitboard_t bbDst = Private::bishopAttackBitboard(b.bbAll, src) & b.bbColor(invert(C));
    while (bbDst) {
      const coord_t dst = SoFUtil::extractLowest(bbDst);
      list[size++] = Move{MoveKind::Simple, src, dst, 0};
    }
  }
  return size;
}

template <Color C>
inline static size_t genRookCapture(const Board &b, Move *list, bitboard_t bbSrc) {
  size_t size = 0;
  while (bbSrc) {
    const coord_t src = SoFUtil::extractLowest(bbSrc);
    bitboard_t bbDst = Private::rookAttackBitboard(b.bbAll, src) & b.bbColor(invert(C));
    while (bbDst) {
      const coord_t dst = SoFUtil::extractLowest(bbDst);
      list[size++] = Move{MoveKind::Simple, src, dst, 0};
    }
  }
  return size;
}

enum class ShiftDirection { Plus, Minus };

template <ShiftDirection Dir>
inline static size_t genDirection(Move *list, size_t size, const coord_t src, const coord_t delta,
                                  const bitboard_t bbSrc, const bitboard_t wallCells,
                                  const bitboard_t cornerCells) {
  coord_t dst = src;
  bitboard_t bbDst = bbSrc;
  while (!(bbDst & cornerCells)) {
    if constexpr (Dir == ShiftDirection::Plus) {
      dst += delta;
      bbDst <<= delta;
    } else {
      dst -= delta;
      bbDst >>= delta;
    }
    if (bbDst & wallCells) {
      break;
    }
    list[size++] = Move{MoveKind::Simple, src, dst, 0};
  }
  return size;
}

inline static size_t genRookSimple(const Board &b, Move *list, bitboard_t bbSrc) {
  using Private::BB_WALL_DOWN;
  using Private::BB_WALL_LEFT;
  using Private::BB_WALL_RIGHT;
  using Private::BB_WALL_UP;

  size_t size = 0;
  while (bbSrc) {
    const coord_t src = SoFUtil::extractLowest(bbSrc);
    const bitboard_t bbSrc = coordToBitboard(src);
    // Go left
    size = genDirection<ShiftDirection::Minus>(list, size, src, 1, bbSrc, b.bbAll, BB_WALL_LEFT);
    // Go up
    size = genDirection<ShiftDirection::Minus>(list, size, src, 8, bbSrc, b.bbAll, BB_WALL_UP);
    // Go right
    size = genDirection<ShiftDirection::Plus>(list, size, src, 1, bbSrc, b.bbAll, BB_WALL_RIGHT);
    // Go down
    size = genDirection<ShiftDirection::Plus>(list, size, src, 8, bbSrc, b.bbAll, BB_WALL_DOWN);
  }
  return size;
}

inline static size_t genBishopSimple(const Board &b, Move *list, bitboard_t bbSrc) {
  using Private::BB_WALL_DOWN;
  using Private::BB_WALL_LEFT;
  using Private::BB_WALL_RIGHT;
  using Private::BB_WALL_UP;

  size_t size = 0;
  while (bbSrc) {
    const coord_t src = SoFUtil::extractLowest(bbSrc);
    const bitboard_t bbSrc = coordToBitboard(src);
    // Go up-left
    size = genDirection<ShiftDirection::Minus>(list, size, src, 9, bbSrc, b.bbAll,
                                               BB_WALL_UP | BB_WALL_LEFT);
    // Go up-right
    size = genDirection<ShiftDirection::Minus>(list, size, src, 7, bbSrc, b.bbAll,
                                               BB_WALL_UP | BB_WALL_RIGHT);
    // Go down-left
    size = genDirection<ShiftDirection::Plus>(list, size, src, 7, bbSrc, b.bbAll,
                                              BB_WALL_DOWN | BB_WALL_LEFT);
    // Go up-right
    size = genDirection<ShiftDirection::Plus>(list, size, src, 9, bbSrc, b.bbAll,
                                              BB_WALL_DOWN | BB_WALL_RIGHT);
  }
  return size;
}

template <Color C>
inline static size_t genCastling(const Board &b, Move *list) {
  size_t size = 0;
  constexpr uint8_t castlingPassShift = (C == Color::White) ? 56 : 0;
  constexpr subcoord_t x = (C == Color::White) ? 7 : 0;
  if (b.isKingsideCastling(C)) {
    constexpr bitboard_t castlingPass = Private::BB_CASTLING_KINGSIDE_PASS << castlingPassShift;
    constexpr coord_t src = makeCoord(x, 4);
    constexpr coord_t tmp = makeCoord(x, 5);
    constexpr coord_t dst = makeCoord(x, 6);
    if (!(castlingPass & b.bbAll) && !isCellAttacked<invert(C)>(b, src) &&
        !isCellAttacked<invert(C)>(b, tmp)) {
      list[size++] = Move{MoveKind::CastlingKingside, src, dst, 0};
    }
  }
  if (b.isQueensideCastling(C)) {
    constexpr bitboard_t castlingPass = Private::BB_CASTLING_QUEENSIDE_PASS << castlingPassShift;
    constexpr coord_t src = makeCoord(x, 4);
    constexpr coord_t tmp = makeCoord(x, 3);
    constexpr coord_t dst = makeCoord(x, 2);
    if (!(castlingPass & b.bbAll) && !isCellAttacked<invert(C)>(b, src) &&
        !isCellAttacked<invert(C)>(b, tmp)) {
      list[size++] = Move{MoveKind::CastlingQueenside, src, dst, 0};
    }
  }
  return size;
}

template <Color C>
inline static bitboard_t bbDiagPieces(const Board &b) {
  return b.bbPieces[makeCell(C, Piece::Bishop)] | b.bbPieces[makeCell(C, Piece::Queen)];
}

template <Color C>
inline static bitboard_t bbLinePieces(const Board &b) {
  return b.bbPieces[makeCell(C, Piece::Rook)] | b.bbPieces[makeCell(C, Piece::Queen)];
}

template <Color C>
inline static size_t genCapturesImpl(const Board &b, Move *list) {
  size_t size = 0;
  size += genPawnCapture<C>(b, list + size);
  size += genPawnEnpassant<C>(b, list + size);
  size += genKing<C, false, true>(b, list + size);
  size += genKnight<C, false, true>(b, list + size);
  size += genBishopCapture<C>(b, list + size, bbDiagPieces<C>(b));
  size += genRookCapture<C>(b, list + size, bbLinePieces<C>(b));
  return size;
}

size_t genCaptures(const Board &b, Move *list) {
  return (b.side == Color::White) ? genCapturesImpl<Color::White>(b, list)
                                  : genCapturesImpl<Color::Black>(b, list);
}

template <Color C>
inline static size_t genAllMovesImpl(const Board &b, Move *list) {
  const bitboard_t bbDiag = bbDiagPieces<C>(b);
  const bitboard_t bbLine = bbLinePieces<C>(b);
  size_t size = 0;
  size += genPawnSimple<C>(b, list + size);
  size += genPawnCapture<C>(b, list + size);
  size += genPawnEnpassant<C>(b, list + size);
  size += genKing<C, true, true>(b, list + size);
  size += genKnight<C, true, true>(b, list + size);
  size += genBishopSimple(b, list + size, bbDiag);
  size += genBishopCapture<C>(b, list + size, bbDiag);
  size += genRookSimple(b, list + size, bbLine);
  size += genRookCapture<C>(b, list + size, bbLine);
  size += genCastling<C>(b, list + size);
  return size;
}

size_t genAllMoves(const Board &b, Move *list) {
  return (b.side == Color::White) ? genAllMovesImpl<Color::White>(b, list)
                                  : genAllMovesImpl<Color::Black>(b, list);
}

template <Color C>
inline static size_t genSimpleMovesImpl(const Board &b, Move *list) {
  const bitboard_t bbDiag = bbDiagPieces<C>(b);
  const bitboard_t bbLine = bbLinePieces<C>(b);
  size_t size = 0;
  size += genPawnSimple<C>(b, list + size);
  size += genKing<C, true, false>(b, list + size);
  size += genKnight<C, true, false>(b, list + size);
  size += genBishopSimple(b, list + size, bbDiag);
  size += genRookSimple(b, list + size, bbLine);
  size += genCastling<C>(b, list + size);
  return size;
}

size_t genSimpleMoves(const Board &b, Move *list) {
  return (b.side == Color::White) ? genSimpleMovesImpl<Color::White>(b, list)
                                  : genSimpleMovesImpl<Color::Black>(b, list);
}

template <Color C>
inline static size_t isMoveValidImpl(const Board &b, Move move) {
  if (unlikely(move.kind == MoveKind::Null)) {
    return false;
  }
  constexpr uint8_t castlingPassShift = (C == Color::White) ? 56 : 0;
  if (unlikely(move.kind == MoveKind::CastlingKingside)) {
    constexpr bitboard_t castlingPass = Private::BB_CASTLING_KINGSIDE_PASS << castlingPassShift;
    return b.isKingsideCastling(C) && !(castlingPass & b.bbAll) &&
           !isCellAttacked<invert(C)>(b, move.src) && !isCellAttacked<invert(C)>(b, move.src + 1);
  }
  if (unlikely(move.kind == MoveKind::CastlingQueenside)) {
    constexpr bitboard_t castlingPass = Private::BB_CASTLING_QUEENSIDE_PASS << castlingPassShift;
    return b.isQueensideCastling(C) && !(castlingPass & b.bbAll) &&
           !isCellAttacked<invert(C)>(b, move.src) && !isCellAttacked<invert(C)>(b, move.src - 1);
  }
  const coord_t src = move.src;
  const coord_t dst = move.dst;
  const cell_t srcCell = b.cells[src];
  const cell_t dstCell = b.cells[dst];
  const bitboard_t bbDst = coordToBitboard(dst);
  if (srcCell == makeCell(C, Piece::Pawn)) {
    if (move.kind == MoveKind::PawnDoubleMove) {
      const bitboard_t bbMustEmpty = (C == Color::White)
                                         ? (static_cast<bitboard_t>(0x0101) << (src - 16))
                                         : (static_cast<bitboard_t>(0x010100) << src);
      return !(b.bbAll & bbMustEmpty);
    }
    if (move.kind == MoveKind::Enpassant) {
      return src + 1 == b.enpassantCoord || src - 1 == b.enpassantCoord;
    }
    constexpr subcoord_t promoteX = (C == Color::White) ? 1 : 6;
    if (move.kind != MoveKind::Promote && coordX(src) == promoteX) {
      return false;
    }
    if (dst == EMPTY_CELL) {
      const coord_t delta = (C == Color::White) ? -8 : 8;
      return dst == src + delta;
    }
    if (cellPieceColor(dstCell) != C) {
      const bitboard_t *attackArr =
          (C == Color::White) ? Private::WHITE_PAWN_ATTACKS : Private::BLACK_PAWN_ATTACKS;
      return attackArr[src] & bbDst;
    }
    return false;
  }
  if (unlikely(move.kind != MoveKind::Simple)) {
    return false;
  }
  if (bbDst & b.bbColor(C)) {
    return false;
  }
  if (srcCell == makeCell(C, Piece::King)) {
    return Private::KING_ATTACKS[src] & bbDst;
  }
  if (srcCell == makeCell(C, Piece::Knight)) {
    return Private::KNIGHT_ATTACKS[src] & bbDst;
  }
  const coord_t coordMin = std::min(src, dst);
  const coord_t coordMax = std::max(src, dst);
  const bitboard_t bbMax = coordToBitboard(coordMax);

#define _CHECK_LINE(line)                              \
  ((bbMax & Private::LINE_##line##_UPPER[coordMin]) && \
   !(b.bbAll & Private::LINE_##line##_UPPER[coordMin] & Private::LINE_##line##_LOWER[coordMax]))

  if (srcCell == makeCell(C, Piece::Bishop)) {
    return _CHECK_LINE(DIAG1) || _CHECK_LINE(DIAG2);
  }
  if (srcCell == makeCell(C, Piece::Rook)) {
    return _CHECK_LINE(VERT) || _CHECK_LINE(HORZ);
  }
  if (srcCell == makeCell(C, Piece::Queen)) {
    return _CHECK_LINE(DIAG1) || _CHECK_LINE(DIAG2) || _CHECK_LINE(VERT) || _CHECK_LINE(HORZ);
  }

#undef _CHECK_LINE

  return false;
}

bool isMoveValid(const Board &b, Move move) {
  return (b.side == Color::White) ? isMoveValidImpl<Color::White>(b, move)
                                  : isMoveValidImpl<Color::Black>(b, move);
}

template bool isCellAttacked<Color::White>(const Board &b, coord_t coord);
template bool isCellAttacked<Color::Black>(const Board &b, coord_t coord);

}  // namespace SoFCore
