#include "movegen.h"

#include <immintrin.h>

#include "core/private/near_attacks.h"

namespace SoFCore {

template <Color C>
bool isCellAttacked_simple(const SoFCore::Board &b, SoFCore::coord_t coord) {
  const cell_t offset = colorOffset(C) - 1;
  const auto *attackMatrix =
      (C == Color::White) ? Private::WHITE_ATTACK_MATRIX : Private::BLACK_ATTACK_MATRIX;
  if ((b.bbPieces[offset + 1] & attackMatrix[coord][1]) ||
      (b.bbPieces[offset + 2] & attackMatrix[coord][2]) ||
      (b.bbPieces[offset + 3] & attackMatrix[coord][3])) {
    return true;
  }
  return false;
}

template <Color C>
bool isCellAttacked_simpleArrs(const SoFCore::Board &b, SoFCore::coord_t coord) {
  const cell_t offset = colorOffset(C) - 1;
  const auto *attackMatrix =
      (C == Color::White) ? Private::WHITE_PAWN_ATTACKS : Private::BLACK_PAWN_ATTACKS;
  if ((b.bbPieces[offset + 1] & attackMatrix[coord]) ||
      (b.bbPieces[offset + 2] & Private::KING_ATTACKS[coord]) ||
      (b.bbPieces[offset + 3] & Private::KNIGHT_ATTACKS[coord])) {
    return true;
  }
  return false;
}

template <Color C>
bool isCellAttacked_sse(const SoFCore::Board &b, SoFCore::coord_t coord) {
  const cell_t offset = colorOffset(C) - 1;
  const auto *attackMatrix =
      (C == Color::White) ? Private::WHITE_ATTACK_MATRIX : Private::BLACK_ATTACK_MATRIX;
  if (b.bbPieces[offset + 1] & attackMatrix[coord][1]) {
    return true;
  }
  const __m128i pieces = *reinterpret_cast<const __m128i *>(b.bbPieces + (offset + 2));
  const __m128i attack = *reinterpret_cast<const __m128i *>(attackMatrix[coord] + 2);
  if (!_mm_testc_si128(pieces, attack)) {
    return true;
  }
  return false;
}

template <Color C>
bool isCellAttacked_avx(const SoFCore::Board &b, SoFCore::coord_t coord) {
  const cell_t offset = colorOffset(C) - 1;
  const auto *attackMatrix =
      (C == Color::White) ? Private::WHITE_ATTACK_MATRIX : Private::BLACK_ATTACK_MATRIX;
  const __m256i pieces = *reinterpret_cast<const __m256i *>(b.bbPieces + offset);
  const __m256i attack = *reinterpret_cast<const __m256i *>(attackMatrix[coord]);
  if (!_mm256_testc_si256(pieces, attack)) {
    return true;
  }
  return false;
}

template bool isCellAttacked_simple<Color::White>(const Board &b, coord_t coord);
template bool isCellAttacked_simple<Color::Black>(const Board &b, coord_t coord);

template bool isCellAttacked_simpleArrs<Color::White>(const Board &b, coord_t coord);
template bool isCellAttacked_simpleArrs<Color::Black>(const Board &b, coord_t coord);

template bool isCellAttacked_sse<Color::White>(const Board &b, coord_t coord);
template bool isCellAttacked_sse<Color::Black>(const Board &b, coord_t coord);

template bool isCellAttacked_avx<Color::White>(const Board &b, coord_t coord);
template bool isCellAttacked_avx<Color::Black>(const Board &b, coord_t coord);

}  // namespace SoFCore
