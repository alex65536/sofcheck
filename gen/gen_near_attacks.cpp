#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "core/types.h"

using namespace SoFCore;

void printBitboard(std::ostream &out, bitboard_t val) {
  out << "0x" << std::hex << std::setw(16) << std::setfill('0') << val << std::dec
      << std::setfill(' ');
}

void printBitboardArray(std::ostream &out, const std::vector<bitboard_t> &array, const char *name) {
  out << "SoFCore::bitboard_t " << name << "[" << std::dec << array.size() << "] = {\n";
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

std::vector<bitboard_t> generateDirected(const int8_t offX[], const int8_t offY[], size_t size) {
  std::vector<bitboard_t> results(64);

  for (cell_t cell = 0; cell < 64; ++cell) {
    bitboard_t bb = 0;
    subcoord_t x = coordX(cell);
    subcoord_t y = coordY(cell);
    for (size_t direction = 0; direction < size; ++direction) {
      subcoord_t nx = x + static_cast<subcoord_t>(offX[direction]);
      subcoord_t ny = y + static_cast<subcoord_t>(offY[direction]);
      if (nx < 8 && ny < 8) {
        bb |= coordToBitboard(makeCoord(nx, ny));
      }
    }
    results[cell] = bb;
  }

  return results;
}

std::vector<bitboard_t> generateKnightAttacks() {
  const int8_t offX[] = {-2, -2, -1, -1, 2, 2, 1, 1};
  const int8_t offY[] = {-1, 1, -2, 2, -1, 1, -2, 2};
  return generateDirected(offX, offY, 8);
}

std::vector<bitboard_t> generateKingAttacks() {
  const int8_t offX[] = {-1, -1, -1, 0, 0, 1, 1, 1};
  const int8_t offY[] = {-1, 0, 1, -1, 1, -1, 0, 1};
  return generateDirected(offX, offY, 8);
}

std::vector<bitboard_t> generateWhitePawnAttacks() {
  const int8_t offX[] = {-1, -1};
  const int8_t offY[] = {-1, 1};
  return generateDirected(offX, offY, 2);
}

std::vector<bitboard_t> generateBlackPawnAttacks() {
  const int8_t offX[] = {1, 1};
  const int8_t offY[] = {-1, 1};
  return generateDirected(offX, offY, 2);
}

void printCombinedMatrix(std::ostream &out, const std::vector<bitboard_t> &pawn,
                         const std::vector<bitboard_t> &knight, const std::vector<bitboard_t> &king,
                         const char *name) {
  out << "alignas(32) SoFCore::bitboard_t " << name << "[64][4] = {\n";
  for (size_t i = 0; i < 64; ++i) {
    out << "    /*" << std::setw(2) << i << "*/ {0x0, ";
    printBitboard(out, pawn[i]);
    out << ", ";
    printBitboard(out, king[i]);
    out << ", ";
    printBitboard(out, knight[i]);
    out << "}";
    if (i + 1 != 64) {
      out << ",";
    }
    out << "\n";
  }
  out << "};\n";
}

void doGenerate(std::ostream &out) {
  out << "#ifndef NEAR_ATTACKS_INCLUDED\n";
  out << "#define NEAR_ATTACKS_INCLUDED\n";
  out << "\n";
  out << "#include \"core/types.h\"\n";
  out << "\n";
  out << "namespace SoFCore {\n";
  out << "namespace Private {\n";
  out << "\n";

  auto knightAttacks = generateKingAttacks();
  auto kingAttacks = generateKingAttacks();
  auto whitePawnAttacks = generateWhitePawnAttacks();
  auto blackPawnAttacks = generateBlackPawnAttacks();

  printBitboardArray(out, kingAttacks, "KING_ATTACKS");
  out << "\n";
  printBitboardArray(out, knightAttacks, "KNIGHT_ATTACKS");
  out << "\n";
  printBitboardArray(out, whitePawnAttacks, "WHITE_PAWN_ATTACKS");
  out << "\n";
  printBitboardArray(out, blackPawnAttacks, "BLACK_PAWN_ATTACKS");
  out << "\n";

  printCombinedMatrix(out, whitePawnAttacks, kingAttacks, knightAttacks, "WHITE_ATTACK_MATRIX");
  out << "\n";
  printCombinedMatrix(out, blackPawnAttacks, kingAttacks, knightAttacks, "BLACK_ATTACK_MATRIX");
  out << "\n";
  out << "} // namespace Private\n";
  out << "} // namespace SoFCore\n";
  out << "\n";

  out << "#endif // NEAR_ATTACKS_INCLUDED\n";
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
