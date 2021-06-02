#include <cstring>
#include <fstream>
#include <iostream>

#include "common.h"
#include "util/fileutil.h"

using SoFUtil::openWriteFile;
using SoFUtil::panic;

int doGenerate(SourcePrinter &printer);

int main(int argc, char **argv) {
  if (argc == 1) {
    SourcePrinter printer(std::cout);
    return doGenerate(printer);
  }
  if (argc == 2 && strcmp(argv[1], "-h") != 0 && strcmp(argv[1], "--help") != 0) {
    auto badFile = [&](auto err) { return panic(std::move(err.description)); };
    std::ofstream file = openWriteFile(argv[1]).okOrErr(badFile);
    SourcePrinter printer(file);
    return doGenerate(printer);
  }
  std::cerr << "Usage: " << argv[0] << " OUT_FILE" << std::endl;
  return 1;
}
