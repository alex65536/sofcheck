#include "core/strutil.h"

#include <cstring>

namespace SoFCore {

std::string moveToStr(const Move move) {
  char str[BUFSZ_MOVE_STR];
  moveToStr(move, str);
  return std::string(str);
}

void moveToStr(const SoFCore::Move move, char *str) {
  if (move.kind == MoveKind::Null) {
    std::strcpy(str, "0000");  // NOLINT: we don't know the size of the buffer
    return;
  }
  *(str++) = ySubToChar(coordY(move.src));
  *(str++) = xSubToChar(coordX(move.src));
  *(str++) = ySubToChar(coordY(move.dst));
  *(str++) = xSubToChar(coordX(move.dst));
  if (move.promote != 0) {
    const char transpos[] = ".pknbrq?";
    *(str++) = transpos[move.promote & 7];
  }
  *str = '\0';
}

const char *fenParseResultToStr(const FenParseResult res) {
  switch (res) {
    case FenParseResult::Ok:
      return "Ok";
    case FenParseResult::ExpectedSpace:
      return "Expected space";
    case FenParseResult::ExpectedUint16:
      return "Expected uint16";
    case FenParseResult::UnexpectedCharacter:
      return "Unexpected character";
    case FenParseResult::BoardRowOverflow:
      return "Too many cells in a row";
    case FenParseResult::BoardRowUnderflow:
      return "Too little cells in a row";
    case FenParseResult::BoardNotEnoughRows:
      return "Too little rows on a board";
    case FenParseResult::BoardTooManyRows:
      return "Too many rows on a board";
    case FenParseResult::CastlingDuplicate:
      return "The same castling type is encountered twice";
    case FenParseResult::CastlingFieldMissing:
      return "Expected castling type, space found";
    case FenParseResult::EnpassantInvalidCell:
      return "Invalid enpassant cell";
    case FenParseResult::RedundantData:
      return "Redundant data in the string";
    case FenParseResult::InternalError:
      return "Internal parser error";
  }
  return "";
}

const char *validateResultToStr(const ValidateResult res) {
  switch (res) {
    case ValidateResult::Ok:
      return "Ok";
    case ValidateResult::BadData:
      return "Bad data in the board";
    case ValidateResult::TooManyPieces:
      return "Board must have no more than 16 pieces on each side";
    case ValidateResult::NoKing:
      return "One of the sides doesn\'t have a king";
    case ValidateResult::TooManyKings:
      return "One of the sides has more than one king";
    case ValidateResult::InvalidEnpassantRow:
      return "Invalid enpassant row";
    case ValidateResult::InvalidPawnPosition:
      return "Pawns cannot stay on the first and the last line";
    case ValidateResult::OpponentKingAttacked:
      return "The opponent king is under attack";
  }
  return "";
}

}  // namespace SoFCore
