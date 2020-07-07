#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include "core/types.h"
#include <string>

namespace SoFCore {

struct Board {
  // Essential fields that indicate the current position
  cell_t cells[64];
  castling_t castling;
  Color side;
  cell_t enpassantCell;
  uint32_t moveCounter;

  // Auxiliary fields that help the move generator to work faster
  // Note that these ones also MUST be filled in order to work correctly
  bitboard_t bbWhite;
  bitboard_t bbBlack;
  alignas(32) bitboard_t bbPieces[16];

  void setInitialPosition();
  bool setFromFen(const char *fen);
  
  void asFen(char *str) const;
  std::string asFen() const;
  void asPretty(char *str) const;
  std::string asPretty() const;
  
  static Board initialPosition();
  
  // Call this method after setting
  void update();

  // Castling helper methods
  inline void clearCastling() { castling = 0; }
  inline void setAllCastling() { castling = CASTLING_ALL; }

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
