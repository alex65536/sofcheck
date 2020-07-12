#include "common.h"

#include <iomanip>

void printBitboard(std::ostream &out, SoFCore::bitboard_t val) {
  out << "0x" << std::hex << std::setw(16) << std::setfill('0') << val << std::dec
      << std::setfill(' ');
}

template <typename T, typename ItemPrinter>
static void printArrayCommon(std::ostream &out, const std::vector<T> &array, ItemPrinter printer) {
  out << "[" << std::dec << array.size() << "] = {\n";
  for (size_t i = 0; i < array.size(); ++i) {
    out << "    /*" << std::setw(2) << i << "*/ ";
    printer(out, array[i]);
    if (i + 1 != array.size()) {
      out << ",";
    }
    out << "\n";
  }
  out << "};\n";
}

void printCoordArray(std::ostream &out, const std::vector<SoFCore::coord_t> &array,
                     const char *name) {
  out << "constexpr coord_t " << name;
  printArrayCommon(out, array, [](std::ostream &out, int x) { out << x; });
}

void printBitboardArray(std::ostream &out, const std::vector<SoFCore::bitboard_t> &array,
                        const char *name) {
  out << "constexpr bitboard_t " << name;
  printArrayCommon(out, array,
                   [](std::ostream &out, SoFCore::bitboard_t bb) { printBitboard(out, bb); });
}
