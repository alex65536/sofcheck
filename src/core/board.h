#ifndef SOF_CORE_BOARD_INCLUDED
#define SOF_CORE_BOARD_INCLUDED

#include <string>

#include "core/types.h"
#include "util/bit.h"
#include "util/result.h"

namespace SoFCore {

enum class FenParseResult {
  Ok,
  ExpectedSpace,
  ExpectedUint16,
  UnexpectedCharacter,
  BoardRowOverflow,
  BoardRowUnderflow,
  BoardNotEnoughRows,
  BoardTooManyRows,
  CastlingDuplicate,
  CastlingFieldMissing,
  EnpassantInvalidCell,
  RedundantData,
  InternalError
};

enum class ValidateResult {
  Ok,
  BadData,
  TooManyPieces,
  NoKing,
  TooManyKings,
  InvalidEnpassantRow,
  InvalidPawnPosition,
  OpponentKingAttacked
};

enum class BoardPrettyStyle { Ascii, Utf8 };

// Recommended buffer sizes for string conversion methods
constexpr size_t BUFSZ_BOARD_FEN = 120;
constexpr size_t BUFSZ_BOARD_PRETTY_ASCII = 120;
constexpr size_t BUFSZ_BOARD_PRETTY_UTF8 = 300;
// Buffer size to be used in `Board::asPretty` regardless of the style
constexpr size_t BUFSZ_BOARD_PRETTY = 300;

// Check that `BUFSZ_BOARD_PRETTY` is the largest
static_assert(BUFSZ_BOARD_PRETTY_ASCII <= BUFSZ_BOARD_PRETTY);
static_assert(BUFSZ_BOARD_PRETTY_UTF8 <= BUFSZ_BOARD_PRETTY);

struct Board {
  static constexpr size_t BB_PIECES_SZ = 15;

  // Essential fields that indicate the current position
  cell_t cells[64];
  uint8_t unused;  // Unused field, required for alignment, must be set to zero
  Color side;
  castling_t castling;
  coord_t enpassantCoord;  // Position of pawn that performed last double move (or `INVALID_COORD`)
  uint16_t moveCounter;
  uint16_t moveNumber;

  // Auxiliary fields that help the move generator to work faster
  //
  // Note that these ones also MUST be filled in order to work correctly. If you fill the essential
  // fields manually, use `update()` method after you finished setting them. Note that the functions
  // in `SoFCore` maintain these fields automatically, so you don't need to use `update()` after
  // calling one of them
  board_hash_t hash;
  bitboard_t bbWhite;
  bitboard_t bbBlack;
  bitboard_t bbAll;
  bitboard_t bbPieces[BB_PIECES_SZ];

  void setInitialPosition();

  FenParseResult setFromFen(const char *fen);

  void asFen(char *fen) const;
  std::string asFen() const;
  void asPretty(char *str, BoardPrettyStyle style = BoardPrettyStyle::Ascii) const;
  std::string asPretty(BoardPrettyStyle style = BoardPrettyStyle::Ascii) const;

  static Board initialPosition();
  static SoFUtil::Result<Board, FenParseResult> fromFen(const char *fen);

  inline constexpr bitboard_t &bbColor(Color c) { return c == Color::White ? bbWhite : bbBlack; }

  inline constexpr const bitboard_t &bbColor(Color c) const {
    return c == Color::White ? bbWhite : bbBlack;
  }

  inline constexpr coord_t kingPos(Color c) const {
    return SoFUtil::getLowest(bbPieces[makeCell(c, Piece::King)]);
  }

  // Validates the board for correctness
  // Non-critical issues (like bad castling flags or bad bitboards) don't result in unsuccessful
  // return value from this function, so it may still return `Ok` in such cases. Note also that this
  // function will call `update()` automatically, so such issues will be corrected.
  ValidateResult validate();

  // Updates the auxilliary fields in the structure and corrects minor issues in essential fields
  // such as invalid castling and enpassant flags
  void update();

  // Helper methods to change castling flags
  inline constexpr void clearCastling() { castling = 0; }
  inline constexpr void setAllCastling() { castling = CASTLING_ALL; }

  inline constexpr bool isAnyCastling() const { return castling != 0; }

  inline constexpr bool isKingsideCastling(Color c) const { return castling & castlingKingside(c); }
  inline constexpr bool isQueensideCastling(Color c) const {
    return castling & castlingQueenside(c);
  }

  inline constexpr void setKingsideCastling(Color c) { castling |= castlingKingside(c); }
  inline constexpr void setQueensideCastling(Color c) { castling |= castlingQueenside(c); }

  inline constexpr void clearKingsideCastling(Color c) { castling &= ~castlingKingside(c); }
  inline constexpr void clearQueensideCastling(Color c) { castling &= ~castlingQueenside(c); }
  inline constexpr void clearCastling(Color c) {
    castling &= ~castlingKingside(c) & ~castlingQueenside(c);
  }

  inline constexpr void flipKingsideCastling(Color c) { castling ^= castlingKingside(c); }
  inline constexpr void flipQueensideCastling(Color c) { castling ^= castlingQueenside(c); }
};

}  // namespace SoFCore

#endif  // SOF_CORE_BOARD_INCLUDED
