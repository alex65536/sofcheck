#include <iostream>
#include <random>
#include <vector>

#include "common.h"
#include "core/private/magic_util.h"
#include "util/bit.h"

using namespace SoFCore;
using SoFCore::Private::MagicType;

template <MagicType M>
bool isValidMagic(coord_t coord, bitboard_t magic) {
  const bitboard_t mask = Private::buildMagicMask<M>(coord);
  const auto shift = SoFUtil::popcount(mask);
  const size_t submaskCnt = 1UL << shift;
  std::vector<bool> used(submaskCnt);
  for (size_t submask = 0; submask < submaskCnt; ++submask) {
    const bitboard_t occupied = SoFUtil::depositBits(submask, mask);
    const size_t idx = (occupied * magic) >> (64 - shift);
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
    bitboard_t magic = 0;
    do {
      magic = genSparseNumber(rnd);
    } while (!isValidMagic<M>(i, magic));
    result[i] = magic;
  }

  return result;
}

template <MagicType M>
std::vector<coord_t> generateShifts() {
  std::vector<coord_t> shifts(64);
  for (coord_t i = 0; i < 64; ++i) {
    shifts[i] = 64 - Private::getMagicMaskBitSize<M>(i);
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
