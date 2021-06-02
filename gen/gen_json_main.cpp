#include <json/json.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <utility>

#include "common.h"
#include "util/fileutil.h"
#include "util/misc.h"

using SoFUtil::openReadFile;
using SoFUtil::openWriteFile;
using SoFUtil::panic;

int doGenerate(SourcePrinter &printer, const Json::Value &json);

int doGenerate(SourcePrinter &printer, std::istream &in) {
  Json::Value json;
  Json::CharReaderBuilder builder;
  std::string errs;
  if (!Json::parseFromStream(builder, in, &json, &errs)) {
    std::cerr << "JSON parse error: " << errs << std::endl;
    return 1;
  }
  return doGenerate(printer, json);
}

int main(int argc, char **argv) {
  // TODO : improve argument parsing
  if (argc == 1) {
    SourcePrinter printer(std::cout);
    return doGenerate(printer, std::cin);
  }
  if (argc == 3) {
    auto badFile = [&](auto err) { return panic(std::move(err.description)); };
    std::ofstream outFile = openWriteFile(argv[1]).okOrErr(badFile);
    std::ifstream inFile = openReadFile(argv[2]).okOrErr(badFile);
    SourcePrinter printer(outFile);
    return doGenerate(printer, inFile);
  }
  std::cerr << "Usage: " << argv[0] << " OUT_FILE IN_FILE" << std::endl;
  return 1;
}
