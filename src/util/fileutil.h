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

#ifndef SOF_UTIL_FILEUTIL_INCLUDED
#define SOF_UTIL_FILEUTIL_INCLUDED

#include <fstream>
#include <string>

#include "util/result.h"

namespace SoFUtil {

struct FileError {
  std::string description;
};

// Opens `std::ifstream` for reading. This is basically a wrapper over the constructor, but returns
// error in `Result` when opening failed
Result<std::ifstream, FileError> openReadFile(const char *path);
Result<std::ifstream, FileError> openReadFile(const std::string &path);

// Opens `std::ofstream` for writing. This is basically a wrapper over the constructor, but returns
// error in `Result` when opening failed
Result<std::ofstream, FileError> openWriteFile(const char *path);
Result<std::ofstream, FileError> openWriteFile(const std::string &path);

}  // namespace SoFUtil

#endif  // SOF_UTIL_FILEUTIL_INCLUDED
