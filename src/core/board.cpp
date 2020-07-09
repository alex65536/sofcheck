#include "board.h"

#include <cstring>
#include "core/strutil.h"

namespace SoFCore {

std::string Board::asFen() const {
  char buf[200];
  asFen(buf);
  return std::string(buf);
}

void Board::asFen(char *str) const {
  (void)str;
  // TODO
}

std::string Board::asPretty() const {
  char buf[200];
  asPretty(buf);
  return std::string(buf);
}

void Board::asPretty(char *str) const {
  for (subcoord_t i = 0; i < 8; ++i) {
    *(str++) = xSubToChar(i);
    *(str++) = '|';
    for (subcoord_t j = 0; j < 8; ++j) {
      *(str++) = cellToChar(cells[makeCoord(i, j)]);
    }
    *(str++) = '\n';
  }
  *(str++) = '-';
  *(str++) = '+';
  for (subcoord_t i = 0; i < 8; ++i) {
    *(str++) = '-';
  }
  *(str++) = '\n';
  *(str++) = side == Color::White ? 'W' : 'B';
  *(str++) = '|';
  for (subcoord_t i = 0; i < 8; ++i) {
    *(str++) = ySubToChar(i);
  }
  *(str++) = '\n';
  *str = '\0';
}

bool Board::setFromFen(const char *fen) {
  (void)fen;
  // TODO
  return false;
}

void Board::setInitialPosition() {
  std::memset(cells, 0, sizeof(cells));

  for (subcoord_t i = 0; i < 8; ++i) {
    cells[makeCoord(1, i)] = makeCell(Color::Black, Piece::Pawn);
  }
  cells[makeCoord(0, 0)] = makeCell(Color::Black, Piece::Rook);
  cells[makeCoord(0, 1)] = makeCell(Color::Black, Piece::Knight);
  cells[makeCoord(0, 2)] = makeCell(Color::Black, Piece::Bishop);
  cells[makeCoord(0, 3)] = makeCell(Color::Black, Piece::Queen);
  cells[makeCoord(0, 4)] = makeCell(Color::Black, Piece::King);
  cells[makeCoord(0, 5)] = makeCell(Color::Black, Piece::Bishop);
  cells[makeCoord(0, 6)] = makeCell(Color::Black, Piece::Knight);
  cells[makeCoord(0, 7)] = makeCell(Color::Black, Piece::Rook);

  for (subcoord_t i = 0; i < 8; ++i) {
    cells[makeCoord(6, i)] = makeCell(Color::White, Piece::Pawn);
  }
  cells[makeCoord(0, 0)] = makeCell(Color::White, Piece::Rook);
  cells[makeCoord(0, 1)] = makeCell(Color::White, Piece::Knight);
  cells[makeCoord(0, 2)] = makeCell(Color::White, Piece::Bishop);
  cells[makeCoord(0, 3)] = makeCell(Color::White, Piece::Queen);
  cells[makeCoord(0, 4)] = makeCell(Color::White, Piece::King);
  cells[makeCoord(0, 5)] = makeCell(Color::White, Piece::Bishop);
  cells[makeCoord(0, 6)] = makeCell(Color::White, Piece::Knight);
  cells[makeCoord(0, 7)] = makeCell(Color::White, Piece::Rook);

  setAllCastling();
  side = Color::White;
  enpassantCell = INVALID_CELL;
  moveCounter = 0;

  update();
}

Board Board::initialPosition() {
  Board board;
  board.setInitialPosition();
  return board;
}

void Board::update() {
  // TODO : validate and correct enpassant cell
  // TODO : validate and correct castling flags

  // Update the bitboards
  bbWhite = 0;
  bbBlack = 0;
  std::memset(bbPieces, 0, sizeof(bbPieces));
  for (coord_t i = 0; i < 64; ++i) {
    cell_t cell = cells[i];
    if (cell == EMPTY_CELL) {
      continue;
    }
    bitboard_t bbAdd = coordToBitboard(i);
    (cellPieceColor(cell) == Color::White ? bbWhite : bbBlack) |= bbAdd;
    bbPieces[cell] |= bbAdd;
  }
}

}
