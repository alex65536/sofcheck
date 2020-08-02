#ifndef SOF_CORE_TYPES_INCLUDED
#define SOF_CORE_TYPES_INCLUDED

#include <cstddef>
#include <cstdint>

namespace SoFCore {

// Bitboard type
using bitboard_t = uint64_t;

// Cell coordinate type
//
// The cells are numbered in the following way:
//
// 8|  0  1  2  3  4  5  6  7
// 7|  8  9 10 11 12 13 14 15
// 6| 16 17 18 19 20 21 22 23
// 5| 24 25 26 27 28 29 30 31
// 4| 32 33 34 35 36 37 38 39
// 3| 40 41 42 43 44 45 46 47
// 2| 48 49 50 51 52 53 54 55
// 1| 56 57 58 59 60 61 62 63
//  +------------------------
//    a  b  c  d  e  f  g  h
//
// This encoding is called Big-Endian Rank-File Mapping on Chess Programming Wiki.
using coord_t = int8_t;

// X, Y subcoordinates type
using subcoord_t = int8_t;

// Cell contents type
using cell_t = int8_t;

// Castling flags type
using castling_t = uint8_t;

// Board hash type
using board_hash_t = uint64_t;

// Cell color type
enum class Color : int8_t { White = 0, Black = 1 };

// Piece kind type
enum class Piece : int8_t { Pawn = 0, King = 1, Knight = 2, Bishop = 3, Rook = 4, Queen = 5 };

// Castling flag constants
constexpr castling_t CASTLING_WHITE_QUEENSIDE = 1;
constexpr castling_t CASTLING_WHITE_KINGSIDE = 2;
constexpr castling_t CASTLING_BLACK_QUEENSIDE = 4;
constexpr castling_t CASTLING_BLACK_KINGSIDE = 8;
constexpr castling_t CASTLING_ALL = 15;

// Full bitboard (i.e. containing all bits set to one)
constexpr bitboard_t BITBOARD_FULL = ~static_cast<bitboard_t>(0);

// Color offsets
constexpr cell_t WHITE_OFFSET = 1;
constexpr cell_t BLACK_OFFSET = 9;

constexpr cell_t EMPTY_CELL = static_cast<cell_t>(0);
constexpr coord_t INVALID_COORD = static_cast<coord_t>(-1);

// Returns the queenside castling flag depending on color
inline constexpr castling_t castlingQueenside(Color c) {
  return c == Color::White ? CASTLING_WHITE_QUEENSIDE : CASTLING_BLACK_QUEENSIDE;
}

// Returns the kingside castling flag depending on color
inline constexpr castling_t castlingKingside(Color c) {
  return c == Color::White ? CASTLING_WHITE_KINGSIDE : CASTLING_BLACK_KINGSIDE;
}

inline constexpr Color invert(Color c) { return static_cast<Color>(static_cast<int8_t>(c) ^ 1); }

inline constexpr coord_t makeCoord(subcoord_t x, subcoord_t y) { return (x << 3) | y; }

// Extracts X subcoordinate from `coord`
inline constexpr subcoord_t coordX(coord_t coord) { return coord >> 3; }

// Extracts Y subcoordinate from `coord`
inline constexpr subcoord_t coordY(coord_t coord) { return coord & 7; }

// Flips X subcoordinate. This is equal to `makeCoord(7 - x, y)`
inline constexpr subcoord_t coordFlipX(coord_t coord) { return coord ^ 56; }

// Flips Y subcoordinate. This is equal to `makeCell(x, 7 - y)`
inline constexpr subcoord_t coordFlipY(coord_t coord) { return coord ^ 7; }

// Returns a bitboard that has only one set bit. This bit corresponds to coordinate `coord`
inline constexpr bitboard_t coordToBitboard(coord_t coord) {
  return static_cast<bitboard_t>(1) << coord;
}

// Returns `true` if the bitboard `bb` has a bit corresponding to coordinate `coord`
inline constexpr bool bitboardHasBit(bitboard_t bb, coord_t coord) { return (bb >> coord) & 1; }

// Returns the cell corresponding to the pawn of given `color`
inline constexpr cell_t colorOffset(Color color) {
  return (color == Color::White) ? WHITE_OFFSET : BLACK_OFFSET;
}

// Returns `true` if the cell has valid contents
inline constexpr bool isCellValid(cell_t c) {
  return c == EMPTY_CELL || (WHITE_OFFSET <= c && c <= WHITE_OFFSET + 5) ||
         (BLACK_OFFSET <= c && c <= BLACK_OFFSET + 5);
}

// Returns the color of the piece in the specified cell `c`. The cell `c` must be valid and must
// not be empty, otherwise the behavior is undefined
inline constexpr Color cellPieceColor(cell_t c) { return c < 8 ? Color::White : Color::Black; }

// Equivalent of `(c != EMPTY_CELL && cellPieceColor(c) == color)`
inline constexpr bool isCellPieceColorEqualTo(cell_t c, Color color) {
  const uint32_t validPieceSet = (color == Color::White) ? 0x7e : 0x7e00;
  return (static_cast<uint32_t>(1) << c) & validPieceSet;
}

// Returns the type of the piece in the specified cell `c`. The cell must be valid and must not be
// empty, otherwise the behavior is undefined
inline constexpr Piece cellPiece(cell_t c) { return static_cast<Piece>((c & 7) - 1); }

inline constexpr cell_t makeCell(Color color, Piece piece) {
  return colorOffset(color) + static_cast<cell_t>(piece);
}

}  // namespace SoFCore

#endif  // SOF_CORE_TYPES_INCLUDED
