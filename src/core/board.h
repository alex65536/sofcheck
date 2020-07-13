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
  EnpassantInvalidCell
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

// Recommended buffer sizes for string conversion methods
constexpr size_t BUFSZ_BOARD_FEN = 120;
constexpr size_t BUFSZ_BOARD_PRETTY = 120;

struct Board {
  static constexpr size_t BB_PIECES_SZ = 15;

  // Essential fields that indicate the current position
  cell_t cells[64];
  uint8_t unused;  // Unused field, needed for alignment
  Color side;
  castling_t castling;
  coord_t enpassantCoord;  // The position of the pawn that performed double move (or INVALID_COORD)
  uint16_t moveCounter;
  uint16_t moveNumber;

  // Auxiliary fields that help the move generator to work faster
  // Note that these ones also MUST be filled in order to work correctly
  board_hash_t hash;
  bitboard_t bbWhite;
  bitboard_t bbBlack;
  bitboard_t bbAll;
  bitboard_t bbPieces[BB_PIECES_SZ];

  void setInitialPosition();

  FenParseResult setFromFen(const char *fen);

  void asFen(char *fen) const;
  std::string asFen() const;
  void asPretty(char *str) const;
  std::string asPretty() const;

  static Board initialPosition();
  static SoFUtil::Result<Board, FenParseResult> fromFen(const char *fen);

  inline constexpr bitboard_t &bbColor(Color c) { return c == Color::White ? bbWhite : bbBlack; }

  inline constexpr const bitboard_t &bbColor(Color c) const {
    return c == Color::White ? bbWhite : bbBlack;
  }

  inline constexpr coord_t kingPos(Color c) const {
    return SoFUtil::getLowest(bbPieces[makeCell(c, Piece::King)]);
  }

  // Validates if the board is correct
  // Non-critical issues (like bad castling flags or bad bitboards) don't result in unsuccessful
  // return value from this function. Note also that this function will call update() automatically,
  // so such issues shall be corrected.
  ValidateResult validate();

  // Call this method after setting the essential fields
  void update();

  // Castling helper methods
  inline void clearCastling() { castling = 0; }
  inline void setAllCastling() { castling = CASTLING_ALL; }

  inline bool isAnyCastling() const { return castling != 0; }

  inline bool isKingsideCastling(Color c) const { return castling & castlingKingside(c); }
  inline bool isQueensideCastling(Color c) const { return castling & castlingQueenside(c); }

  inline void setKingsideCastling(Color c) { castling |= castlingKingside(c); }
  inline void setQueensideCastling(Color c) { castling |= castlingQueenside(c); }

  inline void clearKingsideCastling(Color c) { castling &= ~castlingKingside(c); }
  inline void clearQueensideCastling(Color c) { castling &= ~castlingQueenside(c); }
  inline void clearCastling(Color c) { castling &= ~castlingKingside(c) & ~castlingQueenside(c); }

  inline void flipKingsideCastling(Color c) { castling ^= castlingKingside(c); }
  inline void flipQueensideCastling(Color c) { castling ^= castlingQueenside(c); }
};

}  // namespace SoFCore

#endif  // SOF_CORE_BOARD_INCLUDED
