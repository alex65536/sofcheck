#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "common.h"
#include "core/types.h"

using namespace SoFCore;

std::vector<bitboard_t> generateDir(int8_t dx, int8_t dy) {
  std::vector<bitboard_t> result(64);
  for (coord_t coord = 0; coord < 64; ++coord) {
    subcoord_t x = coordX(coord) + dx;
    subcoord_t y = coordY(coord) + dy;
    while (x < 8 && y < 8) {
      result[coord] |= coordToBitboard(makeCoord(x, y));
      x += dx;
      y += dy;
    }
  }
  return result;
}

void doGenerate(std::ostream &out) {
  out << "#ifndef PART_LINES_INCLUDED\n";
  out << "#define PART_LINES_INCLUDED\n";
  out << "\n";
  out << "#include \"core/types.h\"\n";
  out << "\n";
  out << "namespace SoFCore {\n";
  out << "namespace Private {\n";
  out << "\n";

  printBitboardArray(out, generateDir(-1, 1), "LINE_DIAG1_LOWER");
  out << "\n";
  printBitboardArray(out, generateDir(1, -1), "LINE_DIAG1_UPPER");
  out << "\n";
  printBitboardArray(out, generateDir(-1, -1), "LINE_DIAG2_LOWER");
  out << "\n";
  printBitboardArray(out, generateDir(1, 1), "LINE_DIAG2_UPPER");
  out << "\n";
  printBitboardArray(out, generateDir(-1, 0), "LINE_VERT_LOWER");
  out << "\n";
  printBitboardArray(out, generateDir(1, 0), "LINE_VERT_UPPER");
  out << "\n";
  printBitboardArray(out, generateDir(0, -1), "LINE_HORZ_LOWER");
  out << "\n";
  printBitboardArray(out, generateDir(0, 1), "LINE_HORZ_UPPER");
  out << "\n";

  out << "} // namespace Private\n";
  out << "} // namespace SoFCore\n";
  out << "\n";

  out << "#endif // PART_LINES_INCLUDED\n";
}

int main(int argc, char **argv) {
  if (argc == 1) {
    doGenerate(std::cout);
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "-h") != 0 && strcmp(argv[1], "--help") != 0) {
    std::ofstream file(argv[1]);
    doGenerate(file);
    return 0;
  }
  std::cerr << "usage: " << argv[0] << " OUT_FILE" << std::endl;
  return 0;
}