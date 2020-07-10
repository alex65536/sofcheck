#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include "core/types.h"
#include <string>

namespace SoFCore {

enum class FenParseResult {
  Ok,
  ExpectedSpace,
  ExpectedUnsignedInt,
  UnexpectedCharacter,
  BoardRowOverflow,
  BoardRowUnderflow,
  BoardNotEnoughRows,
  BoardTooManyRows,
  CastlingDuplicate,
  CastlingFieldMissing,
  EnPassantInvalidCell
};
  
struct Board {
  // Essential fields that indicate the current position
  cell_t cells[64];
  castling_t castling;
  Color side;
  cell_t enpassantCoord; // The position of the pawn that performed double move (or INVALID_COORD)
  uint32_t moveCounter;
  uint32_t moveNumber;

  // Auxiliary fields that help the move generator to work faster
  // Note that these ones also MUST be filled in order to work correctly
  bitboard_t bbWhite;
  bitboard_t bbBlack;
  bitboard_t bbPieces[16];

  void setInitialPosition();
  
  FenParseResult setFromFen(const char *fen);
  
  void asFen(char *fen) const;
  std::string asFen() const;
  void asPretty(char *str) const;
  std::string asPretty() const;
  
  static Board initialPosition();
  
  // TODO : add ValidateBoard (the implementation can be taken from Chess256)
  
  // Call this method after setting
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

  inline void flipKingsideCastling(Color c) { castling ^= castlingKingside(c); }
  inline void flipQueensideCastling(Color c) { castling ^= castlingQueenside(c); }
};

}  // namespace SoFCore

#endif  // BOARD_H_INCLUDED
