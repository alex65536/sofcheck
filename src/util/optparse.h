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

#ifndef SOF_UTIL_OPTPARSE_INCLUDED
#define SOF_UTIL_OPTPARSE_INCLUDED

#include <cxxopts.hpp>
#include <string>

#include "util/result.h"

namespace SoFUtil {

// Wrapper for `cxxopts::Options`. It extends the features of `cxxopts`. For example, it allows to
// use long descriptions with word-wrapping in help messages
//
// See also: https://github.com/jarro2783/cxxopts/issues/321
class OptParser {
public:
  OptParser(int argc, const char *const *argv, std::string description);

  // Sets the long description. It is printed in the end of the help message
  void setLongDescription(const std::string &longDescription);

  // Adds options to the option parser. Works in the same way as `cxxopts::Options`
  cxxopts::OptionAdder addOptions(const std::string &group = "");

  // Sets the output width
  void setWidth(size_t width);

  // Returns the help message as string
  std::string help() const;

  // Parses the command line arguments. When the parser is ready to pass the arguments to the main
  // program, returns the parsed arguments. Otherwise, exits from the program
  //
  // Exiting from the program doesn't always mean that the arguments are invalid. For examples, if
  // `-h` flag is specified, the parser just shows help and exits
  cxxopts::ParseResult parse();

private:
  cxxopts::Options inner_;
  std::string description_;
  std::string longDescription_;
  int argc_;
  const char *const *argv_;
  size_t width_ = 78;
};

}  // namespace SoFUtil

#endif  // SOF_UTIL_OPTPARSE_INCLUDED
