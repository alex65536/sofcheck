// This file is part of SoFCheck
//
// Copyright (c) 2021 Alexander Kernozhitsky and SoFCheck contributors
//
// SoFCheck is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SoFCheck is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "eval/feat/feat.h"
#include "util/fileutil.h"
#include "util/misc.h"
#include "util/result.h"
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
