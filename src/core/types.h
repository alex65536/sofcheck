#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <cstdint>

namespace SoFCore {

/*
 * NOTE: the cells are numbered in the following way:
 *
 * 8|  0  1  2  3  4  5  6  7
 * 7|  8  9 10 11 12 13 14 15
 * 6| 16 17 18 19 20 21 22 23
 * 5| 24 25 26 27 28 29 30 31
 * 4| 32 33 34 35 36 37 38 39
 * 3| 40 41 42 43 44 45 46 47
 * 2| 48 49 50 51 52 53 54 55
 * 1| 56 57 58 59 60 61 62 63
 *  +------------------------
 *     a  b  c  d  e  f  g  h
 */

// Bitboard type
typedef uint64_t bitboard_t;

// Cell coordinate type
typedef uint8_t coord_t;

// X, Y subcoordinates type
typedef uint8_t subcoord_t;

// Cell contents type
typedef uint8_t cell_t;

// Castling flags type
typedef uint8_t castling_t;

// Cell color type
enum class Color : uint8_t { White = 0, Black = 1 };

// Piece kind offsets
enum class Piece : uint8_t { Pawn = 0, King = 1, Knight = 2, Bishop = 3, Rook = 4, Queen = 5 };

// Castling flag constants
constexpr castling_t CASTLING_WHITE_QUEENSIDE = 1;
constexpr castling_t CASTLING_WHITE_KINGSIDE = 2;
constexpr castling_t CASTLING_BLACK_QUEENSIDE = 4;
constexpr castling_t CASTLING_BLACK_KINGSIDE = 8;
constexpr castling_t CASTLING_ALL = 15;

inline constexpr castling_t castlingQueenside(Color c) {
  return c == Color::White ? CASTLING_WHITE_QUEENSIDE : CASTLING_BLACK_QUEENSIDE;
}

inline constexpr castling_t castlingKingside(Color c) {
  return c == Color::White ? CASTLING_WHITE_KINGSIDE : CASTLING_BLACK_KINGSIDE;
}

inline constexpr Color invert(Color c) {
  return static_cast<Color>(static_cast<uint8_t>(c) ^ 1);
}

inline constexpr coord_t makeCoord(subcoord_t x, subcoord_t y) { return (x << 3) | y; }

inline constexpr subcoord_t coordX(coord_t coord) { return coord & 7; }

inline constexpr subcoord_t coordY(coord_t coord) { return coord >> 3; }

inline bitboard_t coordToBitboard(cell_t cell) { return static_cast<bitboard_t>(1) << cell; }

// Color offsets
constexpr cell_t WHITE_OFFSET = 1;
constexpr cell_t BLACK_OFFSET = 9;

// Value to indicate an invalid cell
constexpr cell_t INVALID_CELL = static_cast<cell_t>(-1);

constexpr cell_t EMPTY_CELL = static_cast<cell_t>(0);

inline constexpr cell_t colorOffset(Color color) {
  return (color == Color::White) ? WHITE_OFFSET : BLACK_OFFSET;
}

inline constexpr bool cellHasValidContents(cell_t c) {
  return c == EMPTY_CELL || (WHITE_OFFSET <= c && c <= WHITE_OFFSET + 5) ||
         (BLACK_OFFSET <= c && c <= BLACK_OFFSET + 5);
}

// Returns the color of the piece in the specified cell if it's present
// Otherwise, the behavior is undefined
inline constexpr Color cellPieceColor(cell_t c) { return c < 8 ? Color::White : Color::Black; }

// Returns the type of the piece in the specified cell if it's present
// Otherwise, the behavior is undefined
inline constexpr Piece cellPieceKind(cell_t c) { return static_cast<Piece>((c & 7) - 1); }

inline constexpr cell_t makeCell(Color color, Piece piece) {
  return colorOffset(color) + static_cast<uint8_t>(piece);
}

}  // namespace SoFCore

#endif  // TYPES_H_INCLUDED