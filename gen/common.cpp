#include "common.h"

#include <iomanip>

void printBitboard(std::ostream &out, SoFCore::bitboard_t val) {
  out << "0x" << std::hex << std::setw(16) << std::setfill('0') << val << std::dec
      << std::setfill(' ');
}

static size_t log10(size_t val) {
  size_t res = 0;
  size_t pow = 1;
  while (pow <= val) {
    pow *= 10;
    ++res;
  }
  return res - 1;
}

template <typename T, typename ItemPrinter>
static void printArrayCommon(std::ostream &out, const std::vector<T> &array, size_t indent,
                             ItemPrinter printer) {
  out << "[" << std::dec << array.size() << "] = {\n";
  const size_t idxLen = (array.size() <= 1) ? 1 : log10(array.size() - 1) + 1;
  for (size_t i = 0; i < array.size(); ++i) {
    out << std::string(indent + 4, ' ') << "/*" << std::setw(idxLen) << i << "*/ ";
    printer(out, array[i]);
    if (i + 1 != array.size()) {
      out << ",";
    }
    out << "\n";
  }
  out << std::string(indent, ' ') << "};\n";
}

void printCoordArray(std::ostream &out, const std::vector<SoFCore::coord_t> &array,
                     const char *name, size_t indent) {
  out << "constexpr coord_t " << name;
  printArrayCommon(out, array, indent,
                   [](std::ostream &out, const SoFCore::coord_t x) { out << static_cast<int>(x); });
}

void printBitboardArray(std::ostream &out, const std::vector<SoFCore::bitboard_t> &array,
                        const char *name, size_t indent) {
  out << "constexpr bitboard_t " << name;
  printArrayCommon(out, array, indent,
                   [](std::ostream &out, const SoFCore::bitboard_t bb) { printBitboard(out, bb); });
}

void printIntArray(std::ostream &out, const std::vector<int32_t> &array, const char *name,
                   const char *typeName, size_t indent) {
  out << "constexpr " << typeName << " " << name;
  printArrayCommon(out, array, indent, [](std::ostream &out, const int32_t x) { out << x; });
}
