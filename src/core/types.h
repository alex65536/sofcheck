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

// Cell color type
enum class Color : uint8_t {
  White = 0,
  Black = 1
};

inline constexpr Color invertColor(Color c) {
  return static_cast<Color>(static_cast<uint8_t>(c) ^ 1);
}

inline constexpr coord_t makeCoord(subcoord_t x, subcoord_t y) {
  return (x << 3) | y;
}

inline constexpr subcoord_t coordX(coord_t coord) {
  return coord & 7;
}

inline constexpr subcoord_t coordY(coord_t coord) {
  return coord >> 3;
}

inline bitboard_t coordToBitboard(cell_t cell) {
  return static_cast<bitboard_t>(1) << cell;
}

// Color offsets
constexpr cell_t whiteOffset = 1;
constexpr cell_t blackOffset = 9;

// Piece kind offsets
enum class Piece : uint8_t {
  Pawn = 0,
  King = 1,
  Knight = 2,
  Bishop = 3,
  Rook = 4,
  Queen = 5
};

inline constexpr cell_t colorOffset(Color color) {
  return (color == Color::White) ? whiteOffset : blackOffset;
}

inline constexpr cell_t makeEmptyCell() {
  return 0;
}

inline constexpr cell_t makeCell(Color color, Piece piece) {
  return colorOffset(color) + static_cast<uint8_t>(piece);
}

} // namespace SoFCore

#endif // TYPES_H_INCLUDED
