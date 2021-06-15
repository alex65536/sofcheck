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
#include "util/result.h"

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
