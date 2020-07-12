#include <iostream>
#include <random>
#include <vector>

#include "common.h"
#include "core/private/magic_util.h"
#include "util/bit.h"

using namespace SoFCore;  // NOLINT

enum class MagicType { Rook, Bishop };

template <MagicType M>
inline constexpr bitboard_t buildMagicMask(coord_t coord) {
  return (M == MagicType::Rook) ? Private::buildMagicRookMask(coord)
                                : Private::buildMagicBishopMask(coord);
}

template <MagicType M>
bool isValidMagic(coord_t coord, bitboard_t magic) {
  const bitboard_t mask = buildMagicMask<M>(coord);
  const uint8_t shift = SoFUtil::popcount(mask);
  const size_t len = 1UL << shift;
  std::vector<bool> used(len);
  for (size_t i = 0; i < len; ++i) {
    bitboard_t occupied = SoFUtil::depositBits(i, mask);
    size_t idx = (occupied * magic) >> (64 - shift);
    if (used[idx]) {
      return false;
    }
    used[idx] = true;
  }
  return true;
}

template <typename RandGen>
bitboard_t genSparseNumber(RandGen &rnd) {
  bitboard_t res = 0;
  for (size_t i = 0; i < 64; ++i) {
    res <<= 1;
    if (rnd() % 8 == 0) {
      res |= 1;
    }
  }
  return res;
}

template <MagicType M>
std::vector<bitboard_t> generateMagics() {
  std::mt19937_64 rnd(42);  // NOLINT: it's better to get the same magics during different builds,
                            // so the seed is fixed here
  std::vector<bitboard_t> result(64);

  for (coord_t i = 0; i < 64; ++i) {
    bitboard_t magic;
    do {
      magic = genSparseNumber(rnd);
    } while (!isValidMagic<M>(i, magic));
    result[i] = magic;
  }

  return result;
}

template <MagicType M>
std::vector<coord_t> generateShifts() {
  std::vector<SoFCore::coord_t> shifts(64);
  for (coord_t i = 0; i < 64; ++i) {
    shifts[i] = 64 - SoFUtil::popcount(buildMagicMask<M>(i));
  }
  return shifts;
}

void doGenerate(std::ostream &out) {
  out << "#ifndef SOF_CORE_PRIVATE_MAGIC_CONSTANTS_INCLUDED\n";
  out << "#define SOF_CORE_PRIVATE_MAGIC_CONSTANTS_INCLUDED\n";
  out << "\n";
  out << "#include \"core/types.h\"\n";
  out << "\n";
  out << "namespace SoFCore::Private {\n";
  out << "\n";

  printBitboardArray(out, generateMagics<MagicType::Rook>(), "ROOK_MAGICS");
  out << "\n";
  printBitboardArray(out, generateMagics<MagicType::Bishop>(), "BISHOP_MAGICS");
  out << "\n";
  printCoordArray(out, generateShifts<MagicType::Rook>(), "ROOK_SHIFTS");
  out << "\n";
  printCoordArray(out, generateShifts<MagicType::Bishop>(), "BISHOP_SHIFTS");
  out << "\n";

  out << "}  // namespace SoFCore::Private\n";
  out << "\n";

  out << "#endif  // SOF_CORE_PRIVATE_MAGIC_CONSTANTS_INCLUDED\n";
}