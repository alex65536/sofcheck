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

#include <cxxopts.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "eval/feat/feat.h"
#include "util/fileutil.h"
#include "util/misc.h"
#include "util/result.h"

using SoFEval::Feat::Features;
using SoFEval::Feat::weight_t;
using SoFUtil::openReadFile;
using SoFUtil::panic;

constexpr const char *DESCRIPTION =
    "ShowWeights for SoFCheck\n\nThis utility reads the weights from the JSON file with features "
    "and displays them as a list of space-separated integers on the standard output.";

constexpr const char *FEATURES_DESCRIPTION = "JSON file that contains all the evaluation features";

int main(int argc, char **argv) {
  cxxopts::Options optionParser("show_weights", DESCRIPTION);
  optionParser.add_options()           //
      ("h,help", "Show help message")  //
      ("f,features", FEATURES_DESCRIPTION, cxxopts::value<std::string>());
  auto options = optionParser.parse(argc, argv);
  if (options.count("help")) {
    std::cout << optionParser.help() << std::endl;
    return 0;
  }

  const std::string featuresFileName = options["features"].as<std::string>();
  auto badFile = [&](auto err) { return panic(std::move(err.description)); };
  std::ifstream inFile = openReadFile(featuresFileName).okOrErr(badFile);
  const auto features = Features::load(inFile).okOrErr(
      [](const auto err) { panic("Error extracting features: " + err.description); });
  std::vector<weight_t> weights = features.extract();
  for (size_t idx = 0; idx < weights.size(); ++idx) {
    if (idx != 0) {
      std::cout << " ";
    }
    std::cout << weights[idx];
  }
  std::cout << std::endl;
  return 0;
}
