#include <cstring>
#include <fstream>
#include <iostream>

#include "common.h"

int doGenerate(SourcePrinter &printer);

int main(int argc, char **argv) {
  if (argc == 1) {
    SourcePrinter printer(std::cout);
    return doGenerate(printer);
  }
  if (argc == 2 && strcmp(argv[1], "-h") != 0 && strcmp(argv[1], "--help") != 0) {
    std::ofstream file(argv[1]);
    if (!file.is_open()) {
      std::cerr << "Unable to open " << argv[1] << std::endl;
      return 1;
    }
    SourcePrinter printer(file);
    return doGenerate(printer);
  }
  std::cerr << "Usage: " << argv[0] << " OUT_FILE" << std::endl;
  return 1;
}
