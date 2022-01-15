// This file is part of SoFCheck
//
// Copyright (c) 2021-2022 Alexander Kernozhitsky and SoFCheck contributors
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

#include "util/ioutil.h"

#include <charconv>
#include <cstring>
#include <utility>

namespace SoFUtil {

StringWriter::Result<std::monostate> StringWriter::write(const char *data, size_t size) {
  str_ += std::string_view(data, size);
  return Ok(std::monostate{});
}

OstreamWriter::OstreamWriter(std::ostream &out) : out_(&out) {
  out_->rdbuf()->pubsetbuf(nullptr, 0);
}

OstreamWriter::Result<std::monostate> OstreamWriter::write(const char *data, size_t size) {
  if (!out_->write(data, size)) {
    return Err(IOError{"Write error"});
  }
  return Ok(std::monostate{});
}

void BufWriter::flush() {
  doWrite(buffer_.get(), bufPos_);
  bufPos_ = 0;
}

BufWriter &BufWriter::writeStr(const std::string_view &value) {
  if (value.size() >= BUFFER_SIZE) {
    flush();
    doWrite(value.data(), value.size());
    return *this;
  }
  if (bufPos_ + value.size() >= BUFFER_SIZE) {
    const size_t splitPos = BUFFER_SIZE - bufPos_;
    std::memcpy(buffer_.get() + bufPos_, value.data(), splitPos);
    bufPos_ = BUFFER_SIZE;
    flush();
    std::memcpy(buffer_.get(), value.data() + splitPos, value.size() - splitPos);
    bufPos_ = value.size() - splitPos;
    return *this;
  }
  std::memcpy(buffer_.get() + bufPos_, value.data(), value.size());
  bufPos_ += value.size();
  return *this;
}

template <typename Int>
BufWriter &BufWriter::writeInt(const Int value) {
  auto result = std::to_chars(buffer_.get() + bufPos_, buffer_.get() + BUFFER_SIZE, value);
  if (result.ec == std::errc::value_too_large) {
    flush();
    result = std::to_chars(buffer_.get(), buffer_.get() + BUFFER_SIZE, value);
  }
  SOF_ASSERT(result.ec == std::errc());
  bufPos_ = result.ptr - buffer_.get();
  return *this;
}

// Various template specializations for `BufWriter::writeInt()` (for signed and unsigned types)
#define D_WRINT(type)                                  \
  template BufWriter &BufWriter::writeInt(type value); \
  template BufWriter &BufWriter::writeInt(unsigned type value);

D_WRINT(char)
D_WRINT(short)
D_WRINT(int)
D_WRINT(long)
D_WRINT(long long)

#undef D_WRINT

void BufWriter::doWrite(const char *pos, const size_t size) {
  writer_->write(pos, size).okOrErr(
      [](const auto error) { panic("Writer failed: " + error.description); });
}

template <typename F>
inline static Result<F, IOError> openFileCommon(const char *path) {
  F file(path);
  if (!file.is_open()) {
    return Err(IOError{std::string("Unable to open file \"") + path + "\""});
  }
  return Ok(std::move(file));
}

Result<std::ifstream, IOError> openReadFile(const char *path) {
  return openFileCommon<std::ifstream>(path);
}

Result<std::ifstream, IOError> openReadFile(const std::string &path) {
  return openReadFile(path.c_str());
}

Result<std::ofstream, IOError> openWriteFile(const char *path) {
  return openFileCommon<std::ofstream>(path);
}

Result<std::ofstream, IOError> openWriteFile(const std::string &path) {
  return openWriteFile(path.c_str());
}

}  // namespace SoFUtil
