// This file is part of SoFCheck
//
// Copyright (c) 2020-2021 Alexander Kernozhitsky and SoFCheck contributors
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
#include <utility>

#include "common.h"
#include "util/fileutil.h"
#include "util/misc.h"
#include "util/result.h"

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
