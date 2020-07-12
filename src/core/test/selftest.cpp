#include "selftest.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>

#include "core/move.h"
#include "core/movegen.h"

namespace SoFCore::Test {

void testBoardValid(const Board &b) {
  Board copied = b;
  if (copied.validate() != ValidateResult::Ok) {
    throw std::logic_error("Board::validate() reported that the board is invalid");
  }
  if (copied.castling != b.castling) {
    throw std::logic_error("Casting flags are incorrect");
  }
  if (copied.enpassantCoord != b.enpassantCoord) {
    throw std::logic_error("enpassantCoord is incorrect");
  }
  if (copied.bbAll != b.bbAll) {
    throw std::logic_error("bbAll is incorrect");
  }
  if (copied.bbWhite != b.bbWhite) {
    throw std::logic_error("bbWhite is incorrect");
  }
  if (copied.bbBlack != b.bbBlack) {
    throw std::logic_error("bbBlack is incorrect");
  }
  for (cell_t i = 0; i < 16; ++i) {
    if (copied.bbPieces[i] != b.bbPieces[i]) {
      throw std::logic_error("bbPieces[i] is incorrect");
    }
  }
}

static bool boardsBitCompare(const Board &a, const Board &b) {
  return std::memcmp(reinterpret_cast<const void *>(&a), reinterpret_cast<const void *>(&b),
                     sizeof(Board)) == 0;
}

void selfTest(Board b) {
  // Check that the board itself is valid
  testBoardValid(b);

  auto cmpMoves = [](Move a, Move b) { return a.intEncode() < b.intEncode(); };

  // Try to generate moves in total and compare the result if we generate simple moves and captures
  // separately
  Move moves[300];
  size_t moveCnt = genAllMoves(b, moves);
  Move movesSeparate[300];
  size_t moveSeparateCnt = genSimpleMoves(b, movesSeparate);
  moveSeparateCnt += genCaptures(b, movesSeparate + moveSeparateCnt);
  if (moveSeparateCnt != moveCnt) {
    throw std::logic_error(
        "Moves generated by genAllMoves() differ from genSimpleMoves() + genCaptures()");
  }
  std::sort(moves, moves + moveCnt, cmpMoves);
  std::sort(movesSeparate, movesSeparate + moveCnt, cmpMoves);
  if (!std::equal(moves, moves + moveCnt, movesSeparate)) {
    throw std::logic_error(
        "Moves generated by genAllMoves() differ from genSimpleMoves() + genCaptures()");
  }

  // Check that all the generated moves are well-formed
  for (size_t i = 0; i < moveCnt; ++i) {
    if (!moves[i].isWellFormed(b.side)) {
      throw std::logic_error("Moves generated by genAllMoves() are not well-formed");
    }
  }

  // Check that the well-formed move is generated iff isMoveValid returns true
  std::vector<Move> validMoves;
  const MoveKind kinds[] = {MoveKind::Null,
                            MoveKind::Simple,
                            MoveKind::PawnDoubleMove,
                            MoveKind::Enpassant,
                            MoveKind::Promote,
                            MoveKind::CastlingKingside,
                            MoveKind::CastlingQueenside};
  const cell_t promotes[] = {EMPTY_CELL, makeCell(b.side, Piece::Knight),
                             makeCell(b.side, Piece::Bishop), makeCell(b.side, Piece::Rook),
                             makeCell(b.side, Piece::Queen)};
  for (MoveKind kind : kinds) {
    for (cell_t promote : promotes) {
      if ((promote != EMPTY_CELL) != (kind == MoveKind::Promote)) {
        continue;
      }
      for (coord_t src = 0; src < 64; ++src) {
        for (coord_t dst = 0; dst < 64; ++dst) {
          const Move move{kind, src, dst, promote};
          if (!move.isWellFormed(b.side)) {
            continue;
          }
          if (isMoveValid(b, move)) {
            validMoves.push_back(move);
          }
        }
      }
    }
  }
  std::sort(validMoves.begin(), validMoves.end(), cmpMoves);
  if (validMoves.size() != moveCnt || !std::equal(validMoves.begin(), validMoves.end(), moves)) {
    throw std::logic_error("Valid move list and generated move list mismatch");
  }

  for (size_t i = 0; i < moveCnt; ++i) {
    const Move move = moves[i];
    const Board saved = b;
    MovePersistence p = moveMake(b, move);
    if (isMoveLegal(b)) {
      testBoardValid(b);
    }
    moveUnmake(b, move, p);
    if (!boardsBitCompare(b, saved)) {
      throw std::logic_error("Board becomes different after a pair of make/unmake");
    }
  }
}

}  // namespace SoFCore::Test
