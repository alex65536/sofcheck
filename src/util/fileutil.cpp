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

#include "util/fileutil.h"

#include <utility>

namespace SoFUtil {

template <typename F>
inline static Result<F, FileError> openFileCommon(const char *path) {
  F file(path);
  if (!file.is_open()) {
    return Err(FileError{std::string("Unable to open file \"") + path + "\""});
  }
  return Ok(std::move(file));
}

Result<std::ifstream, FileError> openReadFile(const char *path) {
  return openFileCommon<std::ifstream>(path);
}

Result<std::ofstream, FileError> openWriteFile(const char *path) {
  return openFileCommon<std::ofstream>(path);
}

}  // namespace SoFUtil
