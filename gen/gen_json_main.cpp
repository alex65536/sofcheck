#include <json/json.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <utility>

#include "common.h"

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
    std::ofstream outFile(argv[1]);
    std::ifstream inFile(argv[2]);
    if (!outFile.is_open()) {
      std::cerr << "Unable to open " << argv[1] << std::endl;
      return 1;
    }
    if (!inFile.is_open()) {
      std::cerr << "Unable to open " << argv[2] << std::endl;
      return 1;
    }
    SourcePrinter printer(outFile);
    return doGenerate(printer, inFile);
  }
  std::cerr << "Usage: " << argv[0] << " OUT_FILE IN_FILE" << std::endl;
  return 1;
}
