#include "board.h"

#include <cstring>

#include "core/movegen.h"
#include "core/private/rows.h"
#include "core/strutil.h"
#include "util/bit.h"
#include "util/strutil.h"

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
        *(fen++) = static_cast<char>('0' + empty);
        empty = 0;
      }
      *(fen++) = cellToChar(cur);
    }
    if (empty) {
      *(fen++) = static_cast<char>('0' + empty);
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
  fen += SoFUtil::uintSave(moveCounter, fen);
  *(fen++) = ' ';

  // 6. Move number
  fen += SoFUtil::uintSave(moveNumber, fen);

  *(fen++) = '\0';
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
  unused = 0;

  // 1. Parse board cells
  subcoord_t x = 0;
  subcoord_t y = 0;
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
    char lowC = ('A' <= c && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
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
    const char letter = *(fen++);
    _PARSE_CHECK(isValidYChar(letter), FenParseResult::UnexpectedCharacter);
    const char number = *(fen++);
    _PARSE_CHECK(isValidXChar(number), FenParseResult::UnexpectedCharacter);
    const subcoord_t enpassantX = Private::enpassantSrcRow(side);
    _PARSE_CHECK(charToSubX(number) == Private::enpassantDstRow(side),
                 FenParseResult::EnpassantInvalidCell);
    enpassantCoord = makeCoord(enpassantX, charToSubY(letter));
  }
  _PARSE_CONSUME_SPACE();

  // 5. Parse move counter
  int chars = SoFUtil::uintParse(moveCounter, fen);
  _PARSE_CHECK(chars > 0, FenParseResult::ExpectedUint16);
  fen += chars;
  _PARSE_CONSUME_SPACE();

  // 6. Parse move number
  chars = SoFUtil::uintParse(moveNumber, fen);
  _PARSE_CHECK(chars > 0, FenParseResult::ExpectedUint16);

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
  unused = 0;

  update();
}

Board Board::initialPosition() {
  Board board;
  board.setInitialPosition();
  return board;
}

ValidateResult Board::validate() {
  // Check for BadData and InvalidEnpassantRow
  for (cell_t cell : cells) {
    if (!cellHasValidContents(cell)) {
      return ValidateResult::BadData;
    }
  }
  if ((castling & CASTLING_ALL) != castling) {
    return ValidateResult::BadData;
  }
  if (enpassantCoord != INVALID_COORD) {
    if (enpassantCoord > 64) {
      return ValidateResult::BadData;
    }
    if (coordX(enpassantCoord) != Private::enpassantSrcRow(side)) {
      return ValidateResult::InvalidEnpassantRow;
    }
  }

  // Call update() to recalculate bitboards and fix small errors
  update();

  // Check for TooManyPieces, NoKing, TooManyKings
  if (SoFUtil::popcount(bbWhite) > 16 || SoFUtil::popcount(bbBlack) > 16) {
    return ValidateResult::TooManyPieces;
  }
  const bitboard_t bbWhiteKing = bbPieces[makeCell(Color::White, Piece::King)];
  const bitboard_t bbBlackKing = bbPieces[makeCell(Color::Black, Piece::King)];
  if (!bbWhiteKing || !bbBlackKing) {
    return ValidateResult::NoKing;
  }
  if (SoFUtil::popcount(bbWhiteKing) != 1 || SoFUtil::popcount(bbBlackKing) != 1) {
    return ValidateResult::TooManyKings;
  }

  // Check for InvalidPawnPosition
  const bitboard_t bbPawns =
      bbPieces[makeCell(Color::White, Piece::Pawn)] | bbPieces[makeCell(Color::Black, Piece::Pawn)];
  constexpr bitboard_t bbInvalidPawnPos = 0xff000000000000ff;
  if (bbPawns & bbInvalidPawnPos) {
    return ValidateResult::InvalidPawnPosition;
  }

  // Check for OpponentKingAttacked
  if (!isMoveLegal(*this)) {
    return ValidateResult::OpponentKingAttacked;
  }

  return ValidateResult::Ok;
}

void Board::update() {
  // Update invalid enpassant coord
  if (enpassantCoord != INVALID_COORD) {
    const coord_t enpassantPreCoord =
        (side == Color::White) ? enpassantCoord - 8 : enpassantCoord + 8;
    if (cells[enpassantCoord] != makeCell(invert(side), Piece::Pawn) ||
        cells[enpassantPreCoord] != EMPTY_CELL) {
      enpassantCoord = INVALID_COORD;
    }
  }

  // Update invalid castling flags
  for (Color color : {Color::White, Color::Black}) {
    const subcoord_t x = Private::castlingRow(color);
    if (cells[makeCoord(x, 4)] != makeCell(color, Piece::King)) {
      clearCastling(color);
    }
    if (cells[makeCoord(x, 0)] != makeCell(color, Piece::Rook)) {
      clearQueensideCastling(color);
    }
    if (cells[makeCoord(x, 7)] != makeCell(color, Piece::Rook)) {
      clearKingsideCastling(color);
    }
  }

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
