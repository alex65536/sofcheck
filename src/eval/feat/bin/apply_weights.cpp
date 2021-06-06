#include <json/json.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "eval/feat/feat.h"
#include "util/fileutil.h"
#include "util/misc.h"
#include "util/strutil.h"

using SoFEval::Feat::Features;
using SoFEval::Feat::weight_t;
using SoFUtil::openReadFile;
using SoFUtil::openWriteFile;
using SoFUtil::panic;

constexpr const char *DESCRIPTION = R"DESC(
ApplyWeights for SoFCheck

This utility reads the weights as a list of space-separated integers from the
standard input and updates the JSON file with features using these weights.

Usage: apply_weights FEATURES

  FEATURES  The JSON file that contains all the evaluation features. This file
            will be updated with new features
)DESC";

void showUsage() { std::cerr << SoFUtil::trimEolLeft(DESCRIPTION) << std::flush; }

std::vector<weight_t> readWeights(const Features &features) {
  std::vector<weight_t> coefs(features.count());
  for (auto &coef : coefs) {
    if (!(std::cin >> coef)) {
      panic("Error reading weights");
    }
  }
  return coefs;
}

int main(int argc, char **argv) {
  if (argc == 2 && std::strcmp(argv[1], "-h") == 0) {
    showUsage();
    return 0;
  }
  if (argc != 2) {
    showUsage();
    return 1;
  }
  auto badFile = [&](auto err) { return panic(std::move(err.description)); };
  std::ifstream inFile = openReadFile(argv[1]).okOrErr(badFile);
  auto features = Features::load(inFile).okOrErr(
      [](const auto err) { panic("Error extracting features: " + err.description); });
  inFile.close();
  features.apply(readWeights(features));
  std::ofstream outFile = openWriteFile(argv[1]).okOrErr(badFile);
  features.print(outFile);
  return 0;
}
