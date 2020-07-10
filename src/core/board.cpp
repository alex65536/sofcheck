#include "board.h"

#include <cstring>

#include "core/strutil.h"

namespace SoFCore {

std::string Board::asFen() const {
  char buf[200];
  asFen(buf);
  return std::string(buf);
}

void Board::asFen(char *fen) const {
  // 1. Board cells
  for (subcoord_t i = 0; i < 8; ++i) {
    if (i != 0) {
      *(fen++) = '/';
    }
    int empty = 0;
    for (subcoord_t j = 0; j < 8; ++j) {
      cell_t cur = cells[makeCoord(i, j)];
      if (cur == EMPTY_CELL) {
        ++empty;
        continue;
      }
      if (empty != 0) {
        *(fen++) = '0' + empty;
        empty = 0;
      }
      *(fen++) = cellToChar(cur);
    }
    if (empty) {
      *(fen++) = '0' + empty;
    }
  }
  *(fen++) = ' ';

  // 2. Move side
  *(fen++) = side == Color::White ? 'w' : 'b';
  *(fen++) = ' ';

  // 3. Castling
  if (!isAnyCastling()) {
    *(fen++) = '-';
  } else {
    if (isKingsideCastling(Color::White)) {
      *(fen++) = 'K';
    }
    if (isQueensideCastling(Color::White)) {
      *(fen++) = 'Q';
    }
    if (isKingsideCastling(Color::Black)) {
      *(fen++) = 'k';
    }
    if (isQueensideCastling(Color::Black)) {
      *(fen++) = 'q';
    }
  }
  *(fen++) = ' ';

  // 4. Enpassant cell
  if (enpassantCoord == INVALID_COORD) {
    *(fen++) = '-';
  } else {
    *(fen++) = ySubToChar(coordY(enpassantCoord));
    *(fen++) = side == Color::White ? '6' : '3';
  }
  *(fen++) = ' ';

  // 5. Move counter
  fen += uintSave(moveCounter, fen);
  *(fen++) = ' ';

  // 6. Move number
  fen += uintSave(moveNumber, fen);

  *(fen++) = '\0';
  return;
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

// Helper macros for FEN parser

#define _PARSE_CHECK(cond, res) \
  {                             \
    if (!(cond)) {              \
      return res;               \
    }                           \
  }

#define _PARSE_ADD_CASTLING(code, color, side)                                     \
  {                                                                                \
    if (c == (code)) {                                                             \
      _PARSE_CHECK(!is##side##Castling(color), FenParseResult::CastlingDuplicate); \
      set##side##Castling(color);                                                  \
      hasCastling = true;                                                          \
      continue;                                                                    \
    }                                                                              \
  }

#define _PARSE_CONSUME_SPACE() _PARSE_CHECK(*(fen++) == ' ', FenParseResult::ExpectedSpace)

FenParseResult Board::setFromFen(const char *fen) {
  // 1. Parse board cells
  subcoord_t x = 0, y = 0;
  coord_t cur = 0;
  while (true) {
    char c = *(fen++);
    if ('1' <= c && c <= '8') {
      subcoord_t add = c - '0';
      _PARSE_CHECK(y + add <= 8, FenParseResult::BoardRowOverflow);
      for (subcoord_t i = 0; i < add; ++i) {
        cells[cur++] = EMPTY_CELL;
      }
      y += add;
      continue;
    }
    if (c == '/') {
      _PARSE_CHECK(y == 8, FenParseResult::BoardRowUnderflow);
      ++x;
      y = 0;
      _PARSE_CHECK(x < 8, FenParseResult::BoardTooManyRows);
      continue;
    }
    if (c == ' ') {
      _PARSE_CHECK(y == 8, FenParseResult::BoardRowUnderflow);
      _PARSE_CHECK(x == 7 && cur == 64, FenParseResult::BoardNotEnoughRows);
      break;
    }
    char lowC = ('A' <= c && c <= 'Z') ? (c - 'A' + 'a') : c;
    Piece piece;
    switch (lowC) {
      case 'p':
        piece = Piece::Pawn;
        break;
      case 'k':
        piece = Piece::King;
        break;
      case 'n':
        piece = Piece::Knight;
        break;
      case 'b':
        piece = Piece::Bishop;
        break;
      case 'r':
        piece = Piece::Rook;
        break;
      case 'q':
        piece = Piece::Queen;
        break;
      default:
        return FenParseResult::UnexpectedCharacter;
    }
    Color color = (c == lowC) ? Color::Black : Color::White;
    cells[cur++] = makeCell(color, piece);
    ++y;
  }

  // 2. Parse move side
  switch (*(fen++)) {
    case 'w':
      side = Color::White;
      break;
    case 'b':
      side = Color::Black;
      break;
    default:
      return FenParseResult::UnexpectedCharacter;
  }
  _PARSE_CONSUME_SPACE();

  // 3. Parse castling
  clearCastling();
  if (*fen == '-') {
    ++fen;
    _PARSE_CONSUME_SPACE();
  } else {
    bool hasCastling = false;
    while (true) {
      char c = *(fen++);
      _PARSE_ADD_CASTLING('K', Color::White, Kingside);
      _PARSE_ADD_CASTLING('Q', Color::White, Queenside);
      _PARSE_ADD_CASTLING('k', Color::Black, Kingside);
      _PARSE_ADD_CASTLING('q', Color::Black, Queenside);
      if (c == ' ') {
        _PARSE_CHECK(hasCastling, FenParseResult::CastlingFieldMissing);
        break;
      }
      return FenParseResult::UnexpectedCharacter;
    }
  }

  // 4. Parse enpassant cell
  if (*fen == '-') {
    enpassantCoord = INVALID_COORD;
    ++fen;
  } else {
    char letter = *(fen++);
    _PARSE_CHECK(isValidYChar(letter), FenParseResult::UnexpectedCharacter);
    char number = *(fen++);
    _PARSE_CHECK(isValidXChar(number), FenParseResult::UnexpectedCharacter);
    if (side == Color::White) {
      _PARSE_CHECK(number == '6', FenParseResult::EnPassantInvalidCell);
      enpassantCoord = makeCoord(3, charToSubY(letter));
    } else {
      _PARSE_CHECK(number == '3', FenParseResult::EnPassantInvalidCell);
      enpassantCoord = makeCoord(4, charToSubY(letter));
    }
  }
  _PARSE_CONSUME_SPACE();

  // 5. Parse move counter
  int chars = uintParse(moveCounter, fen);
  _PARSE_CHECK(chars > 0, FenParseResult::ExpectedUnsignedInt);
  fen += chars;
  _PARSE_CONSUME_SPACE();

  // 6. Parse move number
  chars = uintParse(moveNumber, fen);
  _PARSE_CHECK(chars > 0, FenParseResult::ExpectedUnsignedInt);

  update();
  return FenParseResult::Ok;
}

#undef _PARSE_CHECK
#undef _PARSE_ADD_CASTLING
#undef _PARSE_CONSUME_SPACE

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
  enpassantCoord = INVALID_COORD;
  moveCounter = 0;
  moveNumber = 1;

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
  bbAll = bbWhite | bbBlack;
}

}  // namespace SoFCore
