#include <jsoncpp/json/json.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <utility>

int doGenerate(std::ostream &out, const Json::Value &json);

int doGenerate(std::ostream &out, std::istream &in) {
  Json::Value json;
  Json::CharReaderBuilder builder;
  std::string errs;
  if (!Json::parseFromStream(builder, in, &json, &errs)) {
    std::cerr << "JSON parse error: " << errs << std::endl;
    return 1;
  }
  return doGenerate(out, json);
}

int main(int argc, char **argv) {
  // TODO : improve argument parsing
  if (argc == 1) {
    return doGenerate(std::cout, std::cin);
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
    return doGenerate(outFile, inFile);
  }
  std::cerr << "Usage: " << argv[0] << " OUT_FILE IN_FILE" << std::endl;
  return 1;
}
