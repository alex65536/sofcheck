#include "core/move.h"

#include <cstdlib>

#include "core/board.h"
#include "core/private/bit_consts.h"
#include "core/private/rows.h"
#include "core/private/zobrist.h"

namespace SoFCore {

bool Move::isWellFormed(Color c) const {
  if (src >= 64 || dst >= 64) {
    return false;
  }
  if (kind != MoveKind::Null && src == dst) {
    return false;
  }
  switch (kind) {
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
      if (coordX(src) != Private::doubleMoveSrcRow(c) ||
          coordX(dst) != Private::doubleMoveDstRow(c) || coordY(src) != coordY(dst)) {
        return false;
      }
      break;
    }
    case MoveKind::Promote: {
      if (coordX(src) != Private::promoteSrcRow(c) || coordX(dst) != Private::promoteDstRow(c)) {
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
    case MoveKind::CastlingKingside: {
      if (promote != 0) {
        return false;
      }
      const subcoord_t x = Private::castlingRow(c);
      if (src != makeCoord(x, 4) || dst != makeCoord(x, 6)) {
        return false;
      }
      break;
    }
    case MoveKind::CastlingQueenside: {
      if (promote != 0) {
        return false;
      }
      const subcoord_t x = Private::castlingRow(c);
      if (src != makeCoord(x, 4) || dst != makeCoord(x, 2)) {
        return false;
      }
      break;
    }
    case MoveKind::Null: {
      if (src != 0 || dst != 0 || promote != 0) {
        return false;
      }
      break;
    }
    case MoveKind::Enpassant: {
      if (promote != 0) {
        return false;
      }
      if (coordX(src) != Private::enpassantSrcRow(c) ||
          coordX(dst) != Private::enpassantDstRow(c)) {
        return false;
      }
      const subcoord_t srcY = coordY(src);
      const subcoord_t dstY = coordY(dst);
      if (srcY + 1 != dstY && srcY - 1 != dstY) {
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

inline static void updateCastling(Board &b, const bitboard_t bbChange) {
  castling_t castlingMask = CASTLING_ALL;
  if (bbChange & Private::BB_CASTLING_KINGSIDE_SRCS) {
    castlingMask ^= CASTLING_BLACK_KINGSIDE;
  }
  if (bbChange & Private::BB_CASTLING_QUEENSIDE_SRCS) {
    castlingMask ^= CASTLING_BLACK_QUEENSIDE;
  }
  if (bbChange & (Private::BB_CASTLING_KINGSIDE_SRCS << 56)) {
    castlingMask ^= CASTLING_WHITE_KINGSIDE;
  }
  if (bbChange & (Private::BB_CASTLING_QUEENSIDE_SRCS << 56)) {
    castlingMask ^= CASTLING_WHITE_QUEENSIDE;
  }
  b.hash ^= Private::g_zobristCastling[b.castling];
  b.castling &= castlingMask;
  b.hash ^= Private::g_zobristCastling[b.castling];
}

template <Color C, bool Inverse>
inline static void makeKingsideCastling(Board &b) {
  constexpr coord_t firstRowStart = Private::castlingRow(C) << 3;
  constexpr cell_t king = makeCell(C, Piece::King);
  constexpr cell_t rook = makeCell(C, Piece::Rook);
  if constexpr (Inverse) {
    b.cells[firstRowStart + 4] = king;
    b.cells[firstRowStart + 5] = EMPTY_CELL;
    b.cells[firstRowStart + 6] = EMPTY_CELL;
    b.cells[firstRowStart + 7] = rook;
  } else {
    b.cells[firstRowStart + 4] = EMPTY_CELL;
    b.cells[firstRowStart + 5] = rook;
    b.cells[firstRowStart + 6] = king;
    b.cells[firstRowStart + 7] = EMPTY_CELL;
    b.hash ^= Private::g_zobristPieceCastlingKingside[static_cast<size_t>(C)];
  }
  b.bbColor(C) ^= static_cast<bitboard_t>(0xf0) << firstRowStart;
  b.bbPieces[rook] ^= static_cast<bitboard_t>(0xa0) << firstRowStart;
  b.bbPieces[king] ^= static_cast<bitboard_t>(0x50) << firstRowStart;
  if constexpr (!Inverse) {
    b.hash ^= Private::g_zobristCastling[b.castling];
    b.clearCastling(C);
    b.hash ^= Private::g_zobristCastling[b.castling];
  }
}

template <Color C, bool Inverse>
inline static void makeQueensideCastling(Board &b) {
  constexpr coord_t firstRowStart = Private::castlingRow(C) << 3;
  constexpr cell_t king = makeCell(C, Piece::King);
  constexpr cell_t rook = makeCell(C, Piece::Rook);
  if constexpr (Inverse) {
    b.cells[firstRowStart + 0] = rook;
    b.cells[firstRowStart + 2] = EMPTY_CELL;
    b.cells[firstRowStart + 3] = EMPTY_CELL;
    b.cells[firstRowStart + 4] = king;
  } else {
    b.cells[firstRowStart + 0] = EMPTY_CELL;
    b.cells[firstRowStart + 2] = king;
    b.cells[firstRowStart + 3] = rook;
    b.cells[firstRowStart + 4] = EMPTY_CELL;
    b.hash ^= Private::g_zobristPieceCastlingQueenside[static_cast<size_t>(C)];
  }
  b.bbColor(C) ^= static_cast<bitboard_t>(0x1d) << firstRowStart;
  b.bbPieces[rook] ^= static_cast<bitboard_t>(0x09) << firstRowStart;
  b.bbPieces[king] ^= static_cast<bitboard_t>(0x14) << firstRowStart;
  if constexpr (!Inverse) {
    b.hash ^= Private::g_zobristCastling[b.castling];
    b.clearCastling(C);
    b.hash ^= Private::g_zobristCastling[b.castling];
  }
}

template <Color C, bool Inverse>
inline static void makeEnpassant(Board &b, const Move move, const bitboard_t bbChange) {
  const coord_t taken = (C == Color::White) ? move.dst + 8 : move.dst - 8;
  const bitboard_t bbTaken = coordToBitboard(taken);
  constexpr cell_t ourPawn = makeCell(C, Piece::Pawn);
  constexpr cell_t enemyPawn = makeCell(invert(C), Piece::Pawn);
  if constexpr (Inverse) {
    b.cells[move.src] = ourPawn;
    b.cells[move.dst] = EMPTY_CELL;
    b.cells[taken] = enemyPawn;
  } else {
    b.cells[move.src] = EMPTY_CELL;
    b.cells[move.dst] = ourPawn;
    b.cells[taken] = EMPTY_CELL;
    b.hash ^= Private::g_zobristPieces[ourPawn][move.src] ^
              Private::g_zobristPieces[ourPawn][move.dst] ^
              Private::g_zobristPieces[enemyPawn][taken];
  }
  b.bbColor(C) ^= bbChange;
  b.bbPieces[ourPawn] ^= bbChange;
  b.bbColor(invert(C)) ^= bbTaken;
  b.bbPieces[enemyPawn] ^= bbTaken;
}

template <Color C, bool Inverse>
inline static void makePawnDoubleMove(Board &b, const Move move, const bitboard_t bbChange) {
  constexpr cell_t pawn = makeCell(C, Piece::Pawn);
  if constexpr (Inverse) {
    b.cells[move.src] = pawn;
    b.cells[move.dst] = EMPTY_CELL;
  } else {
    b.cells[move.src] = EMPTY_CELL;
    b.cells[move.dst] = pawn;
    b.hash ^= Private::g_zobristPieces[pawn][move.src] ^ Private::g_zobristPieces[pawn][move.dst];
  }
  b.bbColor(C) ^= bbChange;
  b.bbPieces[pawn] ^= bbChange;
  if constexpr (!Inverse) {
    b.enpassantCoord = move.dst;
    b.hash ^= Private::g_zobristEnpassant[move.dst];
  }
}

template <Color C>
inline static MovePersistence moveMakeImpl(Board &b, const Move move) {
  MovePersistence p{b.hash, b.castling, b.enpassantCoord, b.moveCounter, b.cells[move.dst]};
  const cell_t srcCell = b.cells[move.src];
  const cell_t dstCell = b.cells[move.dst];
  const bitboard_t bbSrc = coordToBitboard(move.src);
  const bitboard_t bbDst = coordToBitboard(move.dst);
  const bitboard_t bbChange = bbSrc | bbDst;
  if (b.enpassantCoord != INVALID_CELL) {
    b.hash ^= Private::g_zobristEnpassant[b.enpassantCoord];
  }
  b.enpassantCoord = INVALID_CELL;
  switch (move.kind) {
    case MoveKind::Simple: {
      b.cells[move.src] = EMPTY_CELL;
      b.cells[move.dst] = srcCell;
      b.hash ^= Private::g_zobristPieces[srcCell][move.src] ^
                Private::g_zobristPieces[srcCell][move.dst] ^
                Private::g_zobristPieces[dstCell][move.dst];
      b.bbColor(C) ^= bbChange;
      b.bbPieces[srcCell] ^= bbChange;
      b.bbColor(invert(C)) &= ~bbDst;
      b.bbPieces[dstCell] &= ~bbDst;
      updateCastling(b, bbChange);
      break;
    }
    case MoveKind::PawnDoubleMove: {
      makePawnDoubleMove<C, false>(b, move, bbChange);
      break;
    }
    case MoveKind::Promote: {
      b.cells[move.src] = EMPTY_CELL;
      b.cells[move.dst] = move.promote;
      b.hash ^= Private::g_zobristPieces[srcCell][move.src] ^
                Private::g_zobristPieces[move.promote][move.dst] ^
                Private::g_zobristPieces[dstCell][move.dst];
      b.bbColor(C) ^= bbChange;
      b.bbPieces[makeCell(C, Piece::Pawn)] ^= bbSrc;
      b.bbPieces[move.promote] ^= bbDst;
      b.bbColor(invert(C)) &= ~bbDst;
      b.bbPieces[dstCell] &= ~bbDst;
      updateCastling(b, bbChange);
      break;
    }
    case MoveKind::CastlingKingside: {
      makeKingsideCastling<C, false>(b);
      break;
    }
    case MoveKind::CastlingQueenside: {
      makeQueensideCastling<C, false>(b);
      break;
    }
    case MoveKind::Null: {
      // Do nothing, as it is null move
      break;
    }
    case MoveKind::Enpassant: {
      makeEnpassant<C, false>(b, move, bbChange);
      break;
    }
  }
  const bool resetMoveCounter = dstCell != EMPTY_CELL || srcCell == makeCell(C, Piece::Pawn) ||
                                move.kind == MoveKind::Enpassant;
  if (resetMoveCounter) {
    b.moveCounter = 0;
  } else {
    ++b.moveCounter;
  }
  b.side = invert(C);
  b.hash ^= Private::g_zobristMoveSide;
  if constexpr (C == Color::Black) {
    ++b.moveNumber;
  }
  b.bbAll = b.bbWhite | b.bbBlack;
  return p;
}

MovePersistence moveMake(Board &b, const Move move) {
  return (b.side == Color::White) ? moveMakeImpl<Color::White>(b, move)
                                  : moveMakeImpl<Color::Black>(b, move);
}

template <Color C>
void moveUnmakeImpl(Board &b, const Move move, const MovePersistence p) {
  const bitboard_t bbSrc = coordToBitboard(move.src);
  const bitboard_t bbDst = coordToBitboard(move.dst);
  const bitboard_t bbChange = bbSrc | bbDst;
  const cell_t srcCell = b.cells[move.dst];
  const cell_t dstCell = p.dstCell;
  switch (move.kind) {
    case MoveKind::Simple: {
      b.cells[move.src] = srcCell;
      b.cells[move.dst] = dstCell;
      b.bbColor(C) ^= bbChange;
      b.bbPieces[srcCell] ^= bbChange;
      if (dstCell != EMPTY_CELL) {
        b.bbColor(invert(C)) |= bbDst;
        b.bbPieces[dstCell] |= bbDst;
      }
      break;
    }
    case MoveKind::PawnDoubleMove: {
      makePawnDoubleMove<C, true>(b, move, bbChange);
      break;
    }
    case MoveKind::Promote: {
      b.cells[move.src] = makeCell(C, Piece::Pawn);
      b.cells[move.dst] = dstCell;
      b.bbColor(C) ^= bbChange;
      b.bbPieces[makeCell(C, Piece::Pawn)] ^= bbSrc;
      b.bbPieces[move.promote] ^= bbDst;
      if (dstCell != EMPTY_CELL) {
        b.bbColor(invert(C)) |= bbDst;
        b.bbPieces[dstCell] |= bbDst;
      }
      break;
    }
    case MoveKind::CastlingKingside: {
      makeKingsideCastling<C, true>(b);
      break;
    }
    case MoveKind::CastlingQueenside: {
      makeQueensideCastling<C, true>(b);
      break;
    }
    case MoveKind::Null: {
      // Do nothing, as it is null move
      break;
    }
    case MoveKind::Enpassant: {
      makeEnpassant<C, true>(b, move, bbChange);
      break;
    }
  }
  b.hash = p.hash;
  b.castling = p.castling;
  b.enpassantCoord = p.enpassantCoord;
  b.moveCounter = p.moveCounter;
  b.side = C;
  if constexpr (C == Color::Black) {
    --b.moveNumber;
  }
  b.bbAll = b.bbWhite | b.bbBlack;
}

void moveUnmake(Board &b, const Move move, MovePersistence p) {
  return (b.side == Color::Black) ? moveUnmakeImpl<Color::White>(b, move, p)
                                  : moveUnmakeImpl<Color::Black>(b, move, p);
}

}  // namespace SoFCore
