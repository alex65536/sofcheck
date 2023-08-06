// This file is part of SoFCheck
//
// Copyright (c) 2020-2023 Alexander Kernozhitsky and SoFCheck contributors
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

#include "core/movegen.h"

#include "core/bitboard.h"
#include "core/private/between.h"
#include "core/private/bitboard.h"
#include "core/private/geometry.h"
#include "core/private/magic.h"
#include "core/private/near_attacks.h"
#include "util/bit.h"
#include "util/misc.h"

namespace SoFCore {

template <Color C>
inline static bitboard_t bbDiagPieces(const Board &b) {
  return b.bbPieces[makeCell(C, Piece::Bishop)] | b.bbPieces[makeCell(C, Piece::Queen)];
}

template <Color C>
inline static bitboard_t bbLinePieces(const Board &b) {
  return b.bbPieces[makeCell(C, Piece::Rook)] | b.bbPieces[makeCell(C, Piece::Queen)];
}

template <Color C>
bool isCellAttacked(const SoFCore::Board &b, SoFCore::coord_t coord) {
  // Here, we use black attack map for white, as we need to trace the attack from destination piece,
  // not from the source one
  constexpr auto *pawnAttacks =
      (C == Color::White) ? Private::BLACK_PAWN_ATTACKS : Private::WHITE_PAWN_ATTACKS;

  // Check near attacks
  if ((b.bbPieces[makeCell(C, Piece::Pawn)] & pawnAttacks[coord]) ||
      (b.bbPieces[makeCell(C, Piece::King)] & Private::KING_ATTACKS[coord]) ||
      (b.bbPieces[makeCell(C, Piece::Knight)] & Private::KNIGHT_ATTACKS[coord])) {
    return true;
  }

  // Check far attacks
  return (Private::bishopAttackBitboard(b.bbAll, coord) & bbDiagPieces<C>(b)) ||
         (Private::rookAttackBitboard(b.bbAll, coord) & bbLinePieces<C>(b));
}

template <Color C>
bitboard_t cellAttackers(const Board &b, coord_t coord) {
  // Here, we use black attack map for white, as we need to trace the attack from destination piece,
  // not from the source one
  constexpr auto *pawnAttacks =
      (C == Color::White) ? Private::BLACK_PAWN_ATTACKS : Private::WHITE_PAWN_ATTACKS;

  return (b.bbPieces[makeCell(C, Piece::Pawn)] & pawnAttacks[coord]) |
         (b.bbPieces[makeCell(C, Piece::King)] & Private::KING_ATTACKS[coord]) |
         (b.bbPieces[makeCell(C, Piece::Knight)] & Private::KNIGHT_ATTACKS[coord]) |
         (Private::bishopAttackBitboard(b.bbAll, coord) & bbDiagPieces<C>(b)) |
         (Private::rookAttackBitboard(b.bbAll, coord) & bbLinePieces<C>(b));
}

bool isCheck(const Board &b) {
  const Color c = b.side;
  return isCellAttacked(b, b.kingPos(c), invert(c));
}

bool wasMoveLegal(const Board &b) {
  const Color c = b.side;
  return !isCellAttacked(b, b.kingPos(invert(c)), c);
}

enum class PromoteGenPolicy { All, PromoteOnly, NoPromote };

struct SimpleGenFilter {
  static constexpr bool GEN_CASTLING = true;

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  inline bitboard_t filterDst(const bitboard_t bbDst) const { return bbDst; }
};

struct CheckGenFilter {
  static constexpr bool GEN_CASTLING = false;

  bitboard_t bbDstMask;

  inline bitboard_t filterDst(const bitboard_t bbDst) const { return bbDst & bbDstMask; }
};

template <bool IsPromote>
inline static size_t addPawnWithPromote(Move *list, size_t size, const coord_t src,
                                        const coord_t dst) {
  if constexpr (IsPromote) {
    list[size++] = Move{MoveKind::PromoteKnight, src, dst, 0};
    list[size++] = Move{MoveKind::PromoteBishop, src, dst, 0};
    list[size++] = Move{MoveKind::PromoteRook, src, dst, 0};
    list[size++] = Move{MoveKind::PromoteQueen, src, dst, 0};
  } else {
    list[size++] = Move{MoveKind::Simple, src, dst, 0};
  }
  return size;
}

template <Color C, bool IsPromote, typename GenFilter>
inline static size_t doGenPawnSingle(const Board &b, const bitboard_t bbPawns, Move *list,
                                     size_t size, const GenFilter filter) {
  bitboard_t bbPawnDsts = filter.filterDst(advancePawnForward(C, bbPawns) & ~b.bbAll);
  while (bbPawnDsts) {
    const auto dst = static_cast<coord_t>(SoFUtil::extractLowest(bbPawnDsts));
    size = addPawnWithPromote<IsPromote>(list, size, dst - Private::pawnForwardDelta(C), dst);
  }
  return size;
}

template <Color C, typename GenFilter>
inline static size_t doGenPawnDouble(const Board &b, const bitboard_t bbPawns, Move *list,
                                     size_t size, const GenFilter filter) {
  const bitboard_t bbPawnTmps = advancePawnForward(C, bbPawns) & ~b.bbAll;
  bitboard_t bbPawnDsts = filter.filterDst(advancePawnForward(C, bbPawnTmps) & ~b.bbAll);
  while (bbPawnDsts) {
    const auto dst = static_cast<coord_t>(SoFUtil::extractLowest(bbPawnDsts));
    const coord_t src = dst - 2 * Private::pawnForwardDelta(C);
    list[size++] = Move{MoveKind::PawnDoubleMove, src, dst, 0};
  }
  return size;
}

template <Color C, bool IsPromote, typename GenFilter>
inline static size_t doGenPawnCapture(const Board &b, const bitboard_t bbPawns, Move *list,
                                      size_t size, const GenFilter filter) {
  const bitboard_t bbAllowed = b.bbColor(invert(C));
  constexpr coord_t leftDelta = Private::pawnLeftDelta(C);
  constexpr coord_t rightDelta = Private::pawnRightDelta(C);
  {
    // Left capture
    bitboard_t bbPawnDsts = filter.filterDst(advancePawnLeft(C, bbPawns) & bbAllowed);
    while (bbPawnDsts) {
      const auto dst = static_cast<coord_t>(SoFUtil::extractLowest(bbPawnDsts));
      size = addPawnWithPromote<IsPromote>(list, size, dst - leftDelta, dst);
    }
  }
  {
    // Right capture
    bitboard_t bbPawnDsts = filter.filterDst(advancePawnRight(C, bbPawns) & bbAllowed);
    while (bbPawnDsts) {
      const auto dst = static_cast<coord_t>(SoFUtil::extractLowest(bbPawnDsts));
      size = addPawnWithPromote<IsPromote>(list, size, dst - rightDelta, dst);
    }
  }
  return size;
}

template <Color C, PromoteGenPolicy P, typename GenFilter>
inline static size_t genPawnSimple(const Board &b, Move *list, const GenFilter filter) {
  size_t size = 0;
  constexpr bitboard_t bbPromote = BB_ROW[Private::promoteSrcRow(C)];
  constexpr bitboard_t bbDouble = BB_ROW[Private::doubleMoveSrcRow(C)];
  const bitboard_t bbPawns = b.bbPieces[makeCell(C, Piece::Pawn)];
  if constexpr (P == PromoteGenPolicy::All || P == PromoteGenPolicy::NoPromote) {
    size = doGenPawnSingle<C, false>(b, bbPawns & ~bbPromote, list, size, filter);
    size = doGenPawnDouble<C>(b, bbPawns & bbDouble, list, size, filter);
  }
  if constexpr (P == PromoteGenPolicy::All || P == PromoteGenPolicy::PromoteOnly) {
    size = doGenPawnSingle<C, true>(b, bbPawns & bbPromote, list, size, filter);
  }
  return size;
}

template <Color C, typename GenFilter>
inline static size_t genPawnCapture(const Board &b, Move *list, const GenFilter filter) {
  size_t size = 0;
  constexpr bitboard_t bbPromote = BB_ROW[Private::promoteSrcRow(C)];
  const bitboard_t bbPawns = b.bbPieces[makeCell(C, Piece::Pawn)];
  size = doGenPawnCapture<C, false>(b, bbPawns & ~bbPromote, list, size, filter);
  size = doGenPawnCapture<C, true>(b, bbPawns & bbPromote, list, size, filter);
  return size;
}

template <Color C>
inline static size_t genPawnEnpassant(const Board &b, Move *list) {
  const coord_t enpassantCoord = b.enpassantCoord;
  SOF_ASSUME(enpassantCoord == INVALID_COORD || (enpassantCoord >= 0 && enpassantCoord < 64));
  if (enpassantCoord == INVALID_COORD) {
    return 0;
  }
  size_t size = 0;
  const subcoord_t y = coordY(enpassantCoord);
  const coord_t dst = enpassantCoord + Private::pawnForwardDelta(C);
  // We assume that the cell behind the pawn that made double move is empty, so don't check it
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
  return GenSimple ? (GenCaptures ? ~b.bbColor(C) : ~b.bbAll) : b.bbColor(invert(C));
}

template <Color C, Piece P, bool GenSimple, bool GenCaptures, typename GenFilter>
inline static size_t genKnightOrKing(const Board &b, Move *list, const GenFilter filter) {
  static_assert(P == Piece::Knight || P == Piece::King);
  size_t size = 0;
  const bitboard_t bbAllowed = getAllowedMask<C, GenSimple, GenCaptures>(b);
  constexpr auto *attacks = (P == Piece::Knight) ? Private::KNIGHT_ATTACKS : Private::KING_ATTACKS;
  bitboard_t bbSrc = b.bbPieces[makeCell(C, P)];
  while (bbSrc) {
    const auto src = static_cast<coord_t>(SoFUtil::extractLowest(bbSrc));
    bitboard_t bbDst = filter.filterDst(attacks[src] & bbAllowed);
    while (bbDst) {
      const auto dst = static_cast<coord_t>(SoFUtil::extractLowest(bbDst));
      list[size++] = Move{MoveKind::Simple, src, dst, 0};
    }
  }
  return size;
}

template <Color C, bool GenSimple, bool GenCaptures, typename GenFilter>
inline static size_t genKnight(const Board &b, Move *list, const GenFilter filter) {
  return genKnightOrKing<C, Piece::Knight, GenSimple, GenCaptures>(b, list, filter);
}

template <Color C, bool GenSimple, bool GenCaptures>
inline static size_t genKing(const Board &b, Move *list) {
  return genKnightOrKing<C, Piece::King, GenSimple, GenCaptures>(b, list, SimpleGenFilter{});
}

template <Color C, Piece P, bool GenSimple, bool GenCaptures, typename GenFilter>
inline static size_t genBishopOrRook(const Board &b, Move *list, bitboard_t bbSrc,
                                     const GenFilter filter) {
  static_assert(P == Piece::Bishop || P == Piece::Rook);
  size_t size = 0;
  const bitboard_t bbAllowed = getAllowedMask<C, GenSimple, GenCaptures>(b);
  while (bbSrc) {
    const auto src = static_cast<coord_t>(SoFUtil::extractLowest(bbSrc));
    bitboard_t bbDst = (P == Piece::Bishop) ? Private::bishopAttackBitboard(b.bbAll, src)
                                            : Private::rookAttackBitboard(b.bbAll, src);
    bbDst = filter.filterDst(bbDst & bbAllowed);
    while (bbDst) {
      const auto dst = static_cast<coord_t>(SoFUtil::extractLowest(bbDst));
      list[size++] = Move{MoveKind::Simple, src, dst, 0};
    }
  }
  return size;
}

template <Color C, bool GenSimple, bool GenCaptures, typename GenFilter>
inline static size_t genBishop(const Board &b, Move *list, bitboard_t bbSrc,
                               const GenFilter filter) {
  return genBishopOrRook<C, Piece::Bishop, GenSimple, GenCaptures>(b, list, bbSrc, filter);
}

template <Color C, bool GenSimple, bool GenCaptures, typename GenFilter>
inline static size_t genRook(const Board &b, Move *list, bitboard_t bbSrc, const GenFilter filter) {
  return genBishopOrRook<C, Piece::Rook, GenSimple, GenCaptures>(b, list, bbSrc, filter);
}

template <Color C, typename GenFilter>
inline static size_t genCastling(const Board &b, Move *list, const GenFilter) {
  // We don't verify castling under check here. We rely on the fact that `CheckGenFilter` is passed
  // to `genImplInner()` on check, eliminating the call to this function.
  static_assert(GenFilter::GEN_CASTLING);

  size_t size = 0;
  constexpr subcoord_t x = Private::castlingRow(C);
  constexpr coord_t castlingOffset = Private::castlingOffset(C);
  if (b.isKingsideCastling(C)) {
    constexpr bitboard_t castlingPass = Private::BB_CASTLING_KINGSIDE_PASS << castlingOffset;
    constexpr coord_t src = makeCoord(x, 4);
    constexpr coord_t tmp = makeCoord(x, 5);
    constexpr coord_t dst = makeCoord(x, 6);
    if (!(castlingPass & b.bbAll) && !isCellAttacked<invert(C)>(b, tmp)) {
      list[size++] = Move{MoveKind::CastlingKingside, src, dst, 0};
    }
  }
  if (b.isQueensideCastling(C)) {
    constexpr bitboard_t castlingPass = Private::BB_CASTLING_QUEENSIDE_PASS << castlingOffset;
    constexpr coord_t src = makeCoord(x, 4);
    constexpr coord_t tmp = makeCoord(x, 3);
    constexpr coord_t dst = makeCoord(x, 2);
    if (!(castlingPass & b.bbAll) && !isCellAttacked<invert(C)>(b, tmp)) {
      list[size++] = Move{MoveKind::CastlingQueenside, src, dst, 0};
    }
  }
  return size;
}

template <Color C, bool GenSimple, bool GenCaptures, bool GenSimplePromote, typename GenFilter>
inline static size_t genImplInner(const Board &b, Move *list, const GenFilter filter) {
  static_assert(GenSimple || GenCaptures);
  static_assert(GenSimple || !GenSimplePromote);

  size_t size = 0;
  if constexpr (GenSimple) {
    constexpr PromoteGenPolicy promotePolicy =
        GenSimplePromote ? PromoteGenPolicy::All : PromoteGenPolicy::NoPromote;
    size += genPawnSimple<C, promotePolicy>(b, list + size, filter);
  }
  if constexpr (GenCaptures) {
    size += genPawnCapture<C>(b, list + size, filter);
    size += genPawnEnpassant<C>(b, list + size);
  }
  size += genKing<C, GenSimple, GenCaptures>(b, list + size);
  size += genKnight<C, GenSimple, GenCaptures>(b, list + size, filter);
  size += genBishop<C, GenSimple, GenCaptures>(b, list + size, bbDiagPieces<C>(b), filter);
  size += genRook<C, GenSimple, GenCaptures>(b, list + size, bbLinePieces<C>(b), filter);
  if constexpr (GenSimple && GenFilter::GEN_CASTLING) {
    size += genCastling<C>(b, list + size, filter);
  }
  return size;
}

MoveGen::MoveGen(const Board &b) : b_(b), side_(b.side) {
  const coord_t king = b.kingPos(side_);
  const bitboard_t kingAttackers = cellAttackers(b, king, invert(side_));

  if (!kingAttackers) {
    check_ = CheckKind::None;
    return;
  }

  if (!SoFUtil::hasZeroOrOneBit(kingAttackers)) {
    check_ = CheckKind::Double;
    return;
  }

  check_ = CheckKind::Single;
  const auto checker = static_cast<coord_t>(SoFUtil::getLowest(kingAttackers));
  checkMask_ = Private::between(checker, king) | kingAttackers;
}

template <Color C, bool GenSimple, bool GenCaptures, bool GenSimplePromote>
size_t MoveGen::genImpl(Move *list) const {
  static_assert(GenSimple || GenCaptures);
  static_assert(GenSimple || !GenSimplePromote);

  switch (check_) {
    case CheckKind::None: {
      return genImplInner<C, GenSimple, GenCaptures, GenSimplePromote>(b_, list, SimpleGenFilter{});
    }
    case CheckKind::Single: {
      return genImplInner<C, GenSimple, GenCaptures, GenSimplePromote>(b_, list,
                                                                       CheckGenFilter{checkMask_});
    }
    case CheckKind::Double: {
      // Double check or more. We can move only the king.
      return genKing<C, GenSimple, GenCaptures>(b_, list);
    }
  }

  SOF_UNREACHABLE();
}

template <Color C>
size_t MoveGen::genSimplePromotesImpl(Move *list) const {
  switch (check_) {
    case CheckKind::None: {
      return genPawnSimple<C, PromoteGenPolicy::PromoteOnly>(b_, list, SimpleGenFilter{});
    }
    case CheckKind::Single: {
      return genPawnSimple<C, PromoteGenPolicy::PromoteOnly>(b_, list, CheckGenFilter{checkMask_});
    }
    case CheckKind::Double: {
      // Double check or more. We can move only the king.
      return 0;
    }
  }

  SOF_UNREACHABLE();
}

size_t MoveGen::genCaptures(Move *list) const {
  return (side_ == Color::White) ? genImpl<Color::White, false, true, false>(list)
                                 : genImpl<Color::Black, false, true, false>(list);
}

size_t MoveGen::genAllMoves(Move *list) const {
  return (side_ == Color::White) ? genImpl<Color::White, true, true, true>(list)
                                 : genImpl<Color::Black, true, true, true>(list);
}

size_t MoveGen::genSimpleMoves(Move *list) const {
  return (side_ == Color::White) ? genImpl<Color::White, true, false, true>(list)
                                 : genImpl<Color::Black, true, false, true>(list);
}

size_t MoveGen::genSimpleMovesNoPromote(Move *list) const {
  return (side_ == Color::White) ? genImpl<Color::White, true, false, false>(list)
                                 : genImpl<Color::Black, true, false, false>(list);
}

size_t MoveGen::genSimplePromotes(Move *list) const {
  return (side_ == Color::White) ? genSimplePromotesImpl<Color::White>(list)
                                 : genSimplePromotesImpl<Color::Black>(list);
}

template <Color C>
inline static bool isMoveValidImpl(const Board &b, const Move move) {
  if (SOF_UNLIKELY(move.kind == MoveKind::Null)) {
    return false;
  }
  constexpr coord_t castlingOffset = Private::castlingOffset(C);
  if (SOF_UNLIKELY(move.kind == MoveKind::CastlingKingside)) {
    constexpr bitboard_t castlingPass = Private::BB_CASTLING_KINGSIDE_PASS << castlingOffset;
    return b.isKingsideCastling(C) && !(castlingPass & b.bbAll) &&
           !isCellAttacked<invert(C)>(b, move.src) && !isCellAttacked<invert(C)>(b, move.src + 1);
  }
  if (SOF_UNLIKELY(move.kind == MoveKind::CastlingQueenside)) {
    constexpr bitboard_t castlingPass = Private::BB_CASTLING_QUEENSIDE_PASS << castlingOffset;
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
             dst == static_cast<coord_t>(b.enpassantCoord + Private::pawnForwardDelta(C));
    }
    constexpr subcoord_t promoteX = Private::promoteSrcRow(C);
    if (!isMoveKindPromote(move.kind) && coordX(src) == promoteX) {
      return false;
    }
    if (dstCell == EMPTY_CELL) {
      return dst == static_cast<coord_t>(src + Private::pawnForwardDelta(C));
    }
    if (cellPieceColor(dstCell) != C) {
      const bitboard_t *attackArr =
          (C == Color::White) ? Private::WHITE_PAWN_ATTACKS : Private::BLACK_PAWN_ATTACKS;
      return attackArr[src] & bbDst;
    }
    return false;
  }
  if (SOF_UNLIKELY(move.kind != MoveKind::Simple)) {
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
  if (srcCell == makeCell(C, Piece::Bishop)) {
    return Private::bishopAttackBitboard(b.bbAll, src) & bbDst;
  }
  if (srcCell == makeCell(C, Piece::Rook)) {
    return Private::rookAttackBitboard(b.bbAll, src) & bbDst;
  }
  if (srcCell == makeCell(C, Piece::Queen)) {
    return (Private::bishopAttackBitboard(b.bbAll, src) & bbDst) ||
           (Private::rookAttackBitboard(b.bbAll, src) & bbDst);
  }
  return false;
}

template <Color C>
inline static bool isAttackedMaskedImpl(const Board &b, const coord_t kingPos,
                                        const bitboard_t bbAll, const bitboard_t bbOursMask) {
  // Here, we use black attack map for white, as we need to trace the attack from destination piece,
  // not from the source one
  constexpr auto *pawnAttacks =
      (C == Color::White) ? Private::BLACK_PAWN_ATTACKS : Private::WHITE_PAWN_ATTACKS;

  // Check near attacks
  if (((b.bbPieces[makeCell(C, Piece::Pawn)] & pawnAttacks[kingPos]) |
       (b.bbPieces[makeCell(C, Piece::King)] & Private::KING_ATTACKS[kingPos]) |
       (b.bbPieces[makeCell(C, Piece::Knight)] & Private::KNIGHT_ATTACKS[kingPos])) &
      bbOursMask) {
    return true;
  }

  // Check far attacks
  return (Private::bishopAttackBitboard(bbAll, kingPos) & bbDiagPieces<C>(b) & bbOursMask) ||
         (Private::rookAttackBitboard(bbAll, kingPos) & bbLinePieces<C>(b) & bbOursMask);
}

template <Color C>
inline static bool isMoveLegalImpl(const Board &b, const Move move) {
  if (SOF_UNLIKELY(move.kind == MoveKind::Null)) {
    return !isCheck(b);
  }

  const coord_t src = move.src;
  const coord_t dst = move.dst;
  const bitboard_t bbSrc = coordToBitboard(src);
  const bitboard_t bbDst = coordToBitboard(dst);
  const cell_t srcCell = b.cells[src];

  if (srcCell == makeCell(C, Piece::King)) {
    return !isAttackedMaskedImpl<invert(C)>(b, dst, b.bbAll ^ bbSrc, BB_FULL);
  }

  const coord_t king = b.kingPos(C);
  bitboard_t bbAll = (b.bbAll ^ bbSrc) | bbDst;
  bitboard_t bbOursMask = ~bbDst;
  if (move.kind == MoveKind::Enpassant) {
    const bitboard_t bbTmp = advancePawnForward(invert(C), bbDst);
    bbAll ^= bbTmp;
    bbOursMask ^= bbTmp;
  }
  return !isAttackedMaskedImpl<invert(C)>(b, king, bbAll, bbOursMask);
}

bool isMoveLegal(const Board &b, Move move) {
  return (b.side == Color::White) ? isMoveLegalImpl<Color::White>(b, move)
                                  : isMoveLegalImpl<Color::Black>(b, move);
}

bool isMoveValid(const Board &b, const Move move) {
  return (b.side == Color::White) ? isMoveValidImpl<Color::White>(b, move)
                                  : isMoveValidImpl<Color::Black>(b, move);
}

template bool isCellAttacked<Color::White>(const Board &b, coord_t coord);
template bool isCellAttacked<Color::Black>(const Board &b, coord_t coord);

template bitboard_t cellAttackers<Color::White>(const Board &b, coord_t coord);
template bitboard_t cellAttackers<Color::Black>(const Board &b, coord_t coord);

}  // namespace SoFCore
