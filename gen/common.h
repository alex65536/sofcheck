#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <iostream>
#include <iomanip>
#include <vector>
#include "core/types.h"

void printBitboard(std::ostream &out, SoFCore::bitboard_t val) {
  out << "0x" << std::hex << std::setw(16) << std::setfill('0') << val << std::dec
      << std::setfill(' ');
}

void printBitboardArray(std::ostream &out, const std::vector<SoFCore::bitboard_t> &array, const char *name) {
  out << "constexpr bitboard_t " << name << "[" << std::dec << array.size() << "] = {\n";
  for (size_t i = 0; i < array.size(); ++i) {
    out << "    /*" << std::setw(2) << i << "*/ ";
    printBitboard(out, array[i]);
    if (i + 1 != array.size()) {
      out << ",";
    }
    out << "\n";
  }
  out << "};\n";
}

#endif // COMMON_H_INCLUDED
