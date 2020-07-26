#include "core/move_parser.h"

#include "core/private/geometry.h"
#include "core/strutil.h"
#include "util/misc.h"

namespace SoFCore {

ParsedMove ParsedMove::fromStr(const char *first, const char *last) {
  if (last != first + 4 && last != first + 5) {
    return INVALID_PARSED_MOVE;
  }
  // Parse null move (it is written as "0000")
  if (SOF_UNLIKELY(last == first + 4 && std::equal(first, last, "0000"))) {
    return ParsedMove{0, 0, 0, 0};
  }
  if (!isYCharValid(first[0]) || !isXCharValid(first[1]) || !isYCharValid(first[2]) ||
      !isXCharValid(first[3])) {
    return INVALID_PARSED_MOVE;
  }
  ParsedMove result{0, charsToCoord(first[0], first[1]), charsToCoord(first[2], first[3]),
                    EMPTY_CELL};
  if (last == first + 5) {
    switch (first[4]) {
      case 'n':
        result.promote = makeCell(Color::White, Piece::Knight);
        break;
      case 'b':
        result.promote = makeCell(Color::White, Piece::Bishop);
        break;
      case 'r':
        result.promote = makeCell(Color::White, Piece::Rook);
        break;
      case 'q':
        result.promote = makeCell(Color::White, Piece::Queen);
        break;
      default:
        return INVALID_PARSED_MOVE;
    }
  }
  return result;
}

template <Color C>
inline static Move moveFromParsedImpl(const ParsedMove p, const Board &board) {
  // Convert invalid ParsedMove into invalid Move
  if (SOF_UNLIKELY(p == INVALID_PARSED_MOVE)) {
    return Move::invalid();
  }

  // Convert null move
  constexpr ParsedMove nullParsedMove{0, 0, 0, 0};
  if (SOF_UNLIKELY(p == nullParsedMove)) {
    return Move::null();
  }

  // Convert promote
  if (p.promote != 0) {
    return Move{MoveKind::Promote, p.src, p.dst, makeCell(C, cellPiece(p.promote))};
  }

  Move result{MoveKind::Simple, p.src, p.dst, 0};

  // Convert unusual pawn moves (enpassant and double move)
  if (board.cells[p.src] == makeCell(C, Piece::Pawn)) {
    if (coordX(p.src) == Private::doubleMoveSrcRow(C) &&
        coordX(p.dst) == Private::doubleMoveDstRow(C)) {
      result.kind = MoveKind::PawnDoubleMove;
      return result;
    }
    if (coordY(p.src) != coordY(p.dst) && board.cells[p.dst] == EMPTY_CELL) {
      result.kind = MoveKind::Enpassant;
      return result;
    }
  }

  // Convert castling
  if (board.cells[p.src] == makeCell(C, Piece::King)) {
    constexpr subcoord_t x = Private::castlingRow(C);
    if (p.src == makeCoord(x, 4) && p.dst == makeCoord(x, 6)) {
      result.kind = MoveKind::CastlingKingside;
      return result;
    }
    if (p.src == makeCoord(x, 4) && p.dst == makeCoord(x, 2)) {
      result.kind = MoveKind::CastlingQueenside;
      return result;
    }
  }

  return result;
}

Move moveFromParsed(const ParsedMove parsedMove, const Board &board) {
  return (board.side == Color::White) ? moveFromParsedImpl<Color::White>(parsedMove, board)
                                      : moveFromParsedImpl<Color::Black>(parsedMove, board);
}

}  // namespace SoFCore
