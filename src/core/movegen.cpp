#include "core/movegen.h"

#include "core/private/bit_consts.h"
#include "core/private/magic.h"
#include "core/private/near_attacks.h"
#include "core/private/part_lines.h"
#include "core/private/rows.h"
#include "util/bit.h"
#include "util/misc.h"

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
  return (Private::bishopAttackBitboard(b.bbAll, coord) & diagPieces) ||
         (Private::rookAttackBitboard(b.bbAll, coord) & linePieces);
}

template <Color C>
inline static bool isMoveLegalImpl(const Board &b) {
  return !isCellAttacked<C>(b, b.kingPos(invert(C)));
}

bool isMoveLegal(const Board &b) {
  return (b.side == Color::White) ? isMoveLegalImpl<Color::White>(b)
                                  : isMoveLegalImpl<Color::Black>(b);
}

template <Color C>
inline static bool isCheckImpl(const Board &b) {
  return !isCellAttacked<invert(C)>(b, b.kingPos(C));
}

bool isCheck(const Board &b) {
  return (b.side == Color::White) ? isCheckImpl<Color::White>(b) : isCheckImpl<Color::Black>(b);
}

template <Color C>
inline static size_t addPawnWithPromote(Move *list, size_t size, const coord_t src,
                                        const coord_t dst, const subcoord_t x) {
  if (x == Private::promoteSrcRow(C)) {
    list[size++] = Move{MoveKind::Promote, src, dst, makeCell(C, Piece::Knight)};
    list[size++] = Move{MoveKind::Promote, src, dst, makeCell(C, Piece::Bishop)};
    list[size++] = Move{MoveKind::Promote, src, dst, makeCell(C, Piece::Rook)};
    list[size++] = Move{MoveKind::Promote, src, dst, makeCell(C, Piece::Queen)};
  } else {
    list[size++] = Move{MoveKind::Simple, src, dst, 0};
  }
  return size;
}

inline static constexpr coord_t pawnMoveDelta(Color c) { return (c == Color::White) ? -8 : 8; }

template <Color C>
inline static size_t genPawnSimple(const Board &b, Move *list) {
  size_t size = 0;
  bitboard_t bbPawns = b.bbPieces[makeCell(C, Piece::Pawn)];
  while (bbPawns) {
    const coord_t src = SoFUtil::extractLowest(bbPawns);
    const coord_t dst = src + pawnMoveDelta(C);
    // We assume that pawns cannot stay on lines 0 and 7, so don't check it
    if (b.cells[dst] != EMPTY_CELL) {
      continue;
    }
    const coord_t x = coordX(src);
    size = addPawnWithPromote<C>(list, size, src, dst, x);
    if (x == Private::doubleMoveSrcRow(C)) {
      const coord_t dst2 = dst + pawnMoveDelta(C);
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
    if (y != 0 && isCellPieceColorEqualTo(b.cells[leftDst], invert(C))) {
      size = addPawnWithPromote<C>(list, size, src, leftDst, x);
    }
    if (y != 7 && isCellPieceColorEqualTo(b.cells[rightDst], invert(C))) {
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
  const coord_t y = coordY(enpassantCoord);
  const coord_t dst = enpassantCoord + pawnMoveDelta(C);
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

// Determine what cells we can enter based on what we must generate
template <Color C, bool GenSimple, bool GenCaptures>
inline static bitboard_t getAllowedMask(const Board &b) {
  static_assert(GenSimple || GenCaptures);
  return GenSimple ? (GenCaptures ? (BITBOARD_FULL ^ b.bbColor(C)) : (BITBOARD_FULL ^ b.bbAll))
                   : b.bbColor(invert(C));
}

template <Color C, Piece P, bool GenSimple, bool GenCaptures>
inline static size_t genKnightOrKing(const Board &b, Move *list) {
  static_assert(P == Piece::Knight || P == Piece::King);
  size_t size = 0;
  const bitboard_t bbAllowed = getAllowedMask<C, GenSimple, GenCaptures>(b);
  const bitboard_t *attacks =
      (P == Piece::Knight) ? Private::KNIGHT_ATTACKS : Private::KING_ATTACKS;
  bitboard_t bbSrc = b.bbPieces[makeCell(C, P)];
  while (bbSrc) {
    const coord_t src = SoFUtil::extractLowest(bbSrc);
    bitboard_t bbDest = attacks[src] & bbAllowed;
    while (bbDest) {
      const coord_t dst = SoFUtil::extractLowest(bbDest);
      list[size++] = Move{MoveKind::Simple, src, dst, 0};
    }
  }
  return size;
}

template <Color C, bool GenSimple, bool GenCaptures>
inline static size_t genKnight(const Board &b, Move *list) {
  return genKnightOrKing<C, Piece::Knight, GenSimple, GenCaptures>(b, list);
}

template <Color C, bool GenSimple, bool GenCaptures>
inline static size_t genKing(const Board &b, Move *list) {
  return genKnightOrKing<C, Piece::King, GenSimple, GenCaptures>(b, list);
}

template <Color C, Piece P, bool GenSimple, bool GenCaptures>
inline static size_t genBishopOrRook(const Board &b, Move *list, bitboard_t bbSrc) {
  static_assert(P == Piece::Bishop || P == Piece::Rook);
  size_t size = 0;
  const bitboard_t bbAllowed = getAllowedMask<C, GenSimple, GenCaptures>(b);
  while (bbSrc) {
    const coord_t src = SoFUtil::extractLowest(bbSrc);
    bitboard_t bbDst = (P == Piece::Bishop) ? Private::bishopAttackBitboard(b.bbAll, src)
                                            : Private::rookAttackBitboard(b.bbAll, src);
    bbDst &= bbAllowed;
    while (bbDst) {
      const coord_t dst = SoFUtil::extractLowest(bbDst);
      list[size++] = Move{MoveKind::Simple, src, dst, 0};
    }
  }
  return size;
}

template <Color C, bool GenSimple, bool GenCaptures>
inline static size_t genBishop(const Board &b, Move *list, bitboard_t bbSrc) {
  return genBishopOrRook<C, Piece::Bishop, GenSimple, GenCaptures>(b, list, bbSrc);
}

template <Color C, bool GenSimple, bool GenCaptures>
inline static size_t genRook(const Board &b, Move *list, bitboard_t bbSrc) {
  return genBishopOrRook<C, Piece::Rook, GenSimple, GenCaptures>(b, list, bbSrc);
}

template <Color C>
inline static size_t genCastling(const Board &b, Move *list) {
  size_t size = 0;
  constexpr subcoord_t x = Private::castlingRow(C);
  constexpr uint8_t castlingPassShift = x << 3;
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

template <Color C, bool GenSimple, bool GenCaptures>
inline static size_t genImpl(const Board &b, Move *list) {
  static_assert(GenSimple || GenCaptures);
  size_t size = 0;
  if constexpr (GenSimple) {
    size += genPawnSimple<C>(b, list + size);
  }
  if constexpr (GenCaptures) {
    size += genPawnCapture<C>(b, list + size);
    size += genPawnEnpassant<C>(b, list + size);
  }
  size += genKing<C, GenSimple, GenCaptures>(b, list + size);
  size += genKnight<C, GenSimple, GenCaptures>(b, list + size);
  size += genBishop<C, GenSimple, GenCaptures>(b, list + size, bbDiagPieces<C>(b));
  size += genRook<C, GenSimple, GenCaptures>(b, list + size, bbLinePieces<C>(b));
  if constexpr (GenSimple) {
    size += genCastling<C>(b, list + size);
  }
  return size;
}

size_t genCaptures(const Board &b, Move *list) {
  return (b.side == Color::White) ? genImpl<Color::White, false, true>(b, list)
                                  : genImpl<Color::Black, false, true>(b, list);
}

size_t genAllMoves(const Board &b, Move *list) {
  return (b.side == Color::White) ? genImpl<Color::White, true, true>(b, list)
                                  : genImpl<Color::Black, true, true>(b, list);
}

size_t genSimpleMoves(const Board &b, Move *list) {
  return (b.side == Color::White) ? genImpl<Color::White, true, false>(b, list)
                                  : genImpl<Color::Black, true, false>(b, list);
}

template <Color C>
inline static size_t isMoveValidImpl(const Board &b, Move move) {
  if (unlikely(move.kind == MoveKind::Null)) {
    return false;
  }
  constexpr uint8_t castlingPassShift = Private::castlingRow(C) << 3;
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
      return (src + 1 == b.enpassantCoord || src - 1 == b.enpassantCoord) &&
             dst == static_cast<coord_t>(b.enpassantCoord + pawnMoveDelta(C));
    }
    constexpr subcoord_t promoteX = Private::promoteSrcRow(C);
    if (move.kind != MoveKind::Promote && coordX(src) == promoteX) {
      return false;
    }
    if (dstCell == EMPTY_CELL) {
      return dst == static_cast<coord_t>(src + pawnMoveDelta(C));
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

#define D_CHECK_LINE(line)                             \
  ((bbMax & Private::LINE_##line##_UPPER[coordMin]) && \
   !(b.bbAll & Private::LINE_##line##_UPPER[coordMin] & Private::LINE_##line##_LOWER[coordMax]))

  if (srcCell == makeCell(C, Piece::Bishop)) {
    return D_CHECK_LINE(DIAG1) || D_CHECK_LINE(DIAG2);
  }
  if (srcCell == makeCell(C, Piece::Rook)) {
    return D_CHECK_LINE(VERT) || D_CHECK_LINE(HORZ);
  }
  if (srcCell == makeCell(C, Piece::Queen)) {
    return D_CHECK_LINE(DIAG1) || D_CHECK_LINE(DIAG2) || D_CHECK_LINE(VERT) || D_CHECK_LINE(HORZ);
  }

#undef D_CHECK_LINE

  return false;
}

bool isMoveValid(const Board &b, Move move) {
  return (b.side == Color::White) ? isMoveValidImpl<Color::White>(b, move)
                                  : isMoveValidImpl<Color::Black>(b, move);
}

template bool isCellAttacked<Color::White>(const Board &b, coord_t coord);
template bool isCellAttacked<Color::Black>(const Board &b, coord_t coord);

}  // namespace SoFCore
