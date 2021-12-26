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

#include <json/json.h>

#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include "common.h"
#include "util/fileutil.h"
#include "util/misc.h"
#include "util/optparse.h"
#include "util/result.h"

using SoFUtil::openReadFile;
using SoFUtil::openWriteFile;
using SoFUtil::panic;

GeneratorInfo getGeneratorInfo();
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
  auto genInfo = getGeneratorInfo();

  SoFUtil::OptParser parser(argc, argv, genInfo.description);
  parser.addOptions()                                                                    //
      ("i,input", "Input file (stdin if not specified)", cxxopts::value<std::string>())  //
      ("o,output", "Output file (stdout if not specified)", cxxopts::value<std::string>());
  auto options = parser.parse();

  auto badFile = [&](auto err) { return panic(std::move(err.description)); };
  std::ifstream fileIn;
  std::ofstream fileOut;
  std::istream *in = &std::cin;
  std::ostream *out = &std::cout;
  if (options.count("input")) {
    fileIn = openReadFile(options["input"].as<std::string>()).okOrErr(badFile);
    in = &fileIn;
  }
  if (options.count("output")) {
    fileOut = openWriteFile(options["output"].as<std::string>()).okOrErr(badFile);
    out = &fileOut;
  }

  SourcePrinter printer(*out);
  return doGenerate(printer, *in);
}
