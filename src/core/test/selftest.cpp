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

#include "core/test/selftest.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "core/board.h"
#include "core/move.h"
#include "core/move_parser.h"
#include "core/movegen.h"
#include "core/strutil.h"
#include "core/types.h"
#include "util/misc.h"
#include "util/result.h"

namespace SoFCore::Test {

using SoFUtil::panic;

void testBoardValid(const Board &b) {
  Board copied = b;
  if (copied.validate() != ValidateResult::Ok) {
    panic("Board::validate() reported that the board is invalid");
  }
  if (copied.castling != b.castling) {
    panic("Casting flags are incorrect");
  }
  if (copied.enpassantCoord != b.enpassantCoord) {
    panic("enpassantCoord is incorrect");
  }
  if (copied.bbAll != b.bbAll) {
    panic("bbAll is incorrect");
  }
  if (copied.bbWhite != b.bbWhite) {
    panic("bbWhite is incorrect");
  }
  if (copied.bbBlack != b.bbBlack) {
    panic("bbBlack is incorrect");
  }
  for (cell_t i = 0; i < Board::BB_PIECES_SZ; ++i) {
    if (copied.bbPieces[i] != b.bbPieces[i]) {
      panic("bbPieces[" + std::to_string(i) + "] is incorrect");
    }
  }
  if (copied.hash != b.hash) {
    panic("hash is incorrect");
  }
}

static bool boardsBitCompare(const Board &a, const Board &b) {
  return std::memcmp(reinterpret_cast<const void *>(&a), reinterpret_cast<const void *>(&b),
                     sizeof(Board)) == 0;
}

static size_t filterLegalMoves(Board b, Move *moves, size_t origSize, const char *type) {
  size_t newSize = 0;
  for (size_t i = 0; i < origSize; ++i) {
    const Move move = moves[i];
    if (!move.isWellFormed(b.side)) {
      panic("Move \"" + moveToStr(moves[i]) + "\" of type " + type + " is not well-formed");
    }
    if (isMoveLegal(b, move)) {
      moves[newSize++] = move;
    }
  }
  return newSize;
}

template <typename GenFunc>
static size_t genOnlyLegal(GenFunc func, const MoveGen &g, Move *moves, const char *type,
                           size_t limit) {
  const size_t size = (g.*func)(moves);
  if (size > limit) {
    panic("Number of generated moves of type " + std::string(type) + "exceeds limit " +
          std::to_string(limit));
  }
  return filterLegalMoves(g.board(), moves, size, type);
}

void runSelfTest(Board b) {
  // Check that the board itself is valid
  testBoardValid(b);

  // Check that asFen and setFromFen are symmetrical
  char fen[4096];
  std::memset(fen, '?', sizeof(fen));
  b.asFen(fen);
  fen[4095] = '\0';
  if (std::strlen(fen) + 1 > BUFSZ_BOARD_FEN) {
    panic("Buffer constant for FEN is too small");
  }
  auto loadResult = Board::fromFen(fen);
  if (!loadResult.isOk()) {
    panic("Cannot load from the board from its own FEN");
  }
  const Board &loaded = loadResult.ok();
  if (!boardsBitCompare(b, loaded)) {
    panic("Loading the board from FEN produces a different board");
  }

  // Check that asPretty doesn't overflow the buffer
  const std::vector<std::pair<BoardPrettyStyle, size_t>> bufSizes = {
      {BoardPrettyStyle::Ascii, BUFSZ_BOARD_PRETTY_ASCII},
      {BoardPrettyStyle::Utf8, BUFSZ_BOARD_PRETTY_UTF8}};
  for (auto [style, bufSize] : bufSizes) {
    char pretty[4096];
    std::memset(pretty, '?', sizeof(pretty));
    b.asPretty(pretty, style);
    pretty[4095] = '\0';
    if (std::strlen(pretty) + 1 > bufSize) {
      panic("Buffer constant for pretty board is too slow");
    }
  }

  auto cmpMoves = [](Move a, Move b) { return a.asUint() < b.asUint(); };

  // Try to generate moves in total and compare the result if we generate simple moves and captures
  // separately
  MoveGen gen(b);
  Move moves[1500];
  const size_t moveCnt = genOnlyLegal(&MoveGen::genAllMoves, gen, moves, "ALL", BUFSZ_MOVES);
  Move movesSeparate[1500];
  const size_t simpleCnt = genOnlyLegal(&MoveGen::genSimpleMovesNoPromote, gen, movesSeparate,
                                        "SIMPLE_NO_PROMOTE", BUFSZ_MOVES);
  const size_t promoteCnt =
      genOnlyLegal(&MoveGen::genSimplePromotes, gen, movesSeparate + simpleCnt, "PROMOTE",
                   BUFSZ_SIMPLE_PROMOTES);
  const size_t captureCnt =
      genOnlyLegal(&MoveGen::genCaptures, gen, movesSeparate + simpleCnt + promoteCnt, "CAPTURE",
                   BUFSZ_CAPTURES);
  if (simpleCnt + promoteCnt + captureCnt != moveCnt) {
    panic(
        "Moves generated by genAllMoves() differ from genSimpleMovesNoPromote() + "
        "genSimplePromotes() + genCaptures()");
  }
  Move movesAllSimple[1500];
  const size_t allSimpleCnt =
      genOnlyLegal(&MoveGen::genSimpleMoves, gen, movesAllSimple, "SIMPLE", BUFSZ_MOVES);
  if (simpleCnt + promoteCnt != allSimpleCnt) {
    panic(
        "Moves generated by genSimpleMoves() differ from genSimpleMovesNoPromote() + "
        "genSimplePromotes()");
  }
  std::sort(movesSeparate, movesSeparate + simpleCnt + promoteCnt, cmpMoves);
  std::sort(movesAllSimple, movesAllSimple + allSimpleCnt, cmpMoves);
  if (!std::equal(movesSeparate, movesSeparate + allSimpleCnt, movesAllSimple)) {
    panic(
        "Moves generated by genSimpleMoves() differ from genSimpleMovesNoPromote() + "
        "genSimplePromotes()");
  }
  std::sort(moves, moves + moveCnt, cmpMoves);
  std::sort(movesSeparate, movesSeparate + moveCnt, cmpMoves);
  if (!std::equal(moves, moves + moveCnt, movesSeparate)) {
    panic(
        "Moves generated by genAllMoves() differ from genSimpleMovesNoPromote() + "
        "genSimplePromotes() + genCaptures()");
  }

  // Check that move parser works correctly
  for (size_t i = 0; i < moveCnt; ++i) {
    const Move move = moves[i];
    char str[BUFSZ_MOVE_STR];
    moveToStr(move, str);
    const Move move2 = moveParse(str, b);
    if (move != move2) {
      panic("Move \"" + std::string(str) +
            "\" changed after being converted to string and vice versa");
    }
  }

  // Check that a well-formed move is generated by `genAllMoves()` iff isMoveValid returns true
  std::vector<Move> pseudoLegalMoves;
  const MoveKind kinds[] = {MoveKind::Null,
                            MoveKind::Simple,
                            MoveKind::PawnDoubleMove,
                            MoveKind::Enpassant,
                            MoveKind::PromoteKnight,
                            MoveKind::PromoteBishop,
                            MoveKind::PromoteRook,
                            MoveKind::PromoteQueen,
                            MoveKind::CastlingKingside,
                            MoveKind::CastlingQueenside};
  for (MoveKind kind : kinds) {
    for (coord_t src = 0; src < 64; ++src) {
      for (coord_t dst = 0; dst < 64; ++dst) {
        const Move move{kind, src, dst, 0};
        if (!move.isWellFormed(b.side)) {
          continue;
        }
        if (isMoveValid(b, move)) {
          pseudoLegalMoves.push_back(move);
        }
      }
    }
  }

  auto legalMoves = pseudoLegalMoves;
  legalMoves.resize(filterLegalMoves(b, legalMoves.data(), legalMoves.size(), "VALID"));
  std::sort(legalMoves.begin(), legalMoves.end(), cmpMoves);
  if (!std::equal(legalMoves.begin(), legalMoves.end(), moves, moves + moveCnt)) {
    panic("Valid move list and generated move list mismatch");
  }

  // Check that `wasMoveLegal()` and `isMoveLegal()` are identical. Also check that board is valids
  // after making moves.
  for (const auto &move : pseudoLegalMoves) {
    const bool isLegal1 = isMoveLegal(b, move);
    const Board saved = b;
    const MovePersistence p = moveMake(b, move);
    const bool isLegal2 = wasMoveLegal(b);
    if (isLegal1 != isLegal2) {
      panic("Functions isMoveLegal() and wasMoveLegal() yield different result on move \"" +
            moveToStr(move) + "\"");
    }
    if (isLegal2) {
      testBoardValid(b);
    }
    moveUnmake(b, move, p);
    if (!boardsBitCompare(b, saved)) {
      panic("Board becomes different after making and unmaking move \"" + moveToStr(move) + "\"");
    }
  }
}

}  // namespace SoFCore::Test
