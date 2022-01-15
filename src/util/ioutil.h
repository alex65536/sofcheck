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

#ifndef SOF_UTIL_IOUTIL_INCLUDED
#define SOF_UTIL_IOUTIL_INCLUDED

#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <variant>

#include "util/no_copy_move.h"
#include "util/result.h"

namespace SoFUtil {

// Error type for IO operations
struct IOError {
  std::string description;
};

// Basic interface for writers
class Writer : public virtual VirtualNoCopyMove {
public:
  using Ptr = std::unique_ptr<Writer>;

  template <typename T>
  using Result = Result<T, IOError>;

  // Writes `size` bytes, starting from `data`. If an error occurs, it is returned in `Result`
  virtual Result<std::monostate> write(const char *data, size_t size) = 0;
};

// Writer that writes into strings
class StringWriter : public Writer, public virtual NoCopyMove {
public:
  using Ptr = std::unique_ptr<StringWriter>;

  // Returns the underlying string
  std::string &str() { return str_; }
  const std::string &str() const { return str_; }

  StringWriter() = default;
  static Ptr create() { return std::make_unique<StringWriter>(); }
  Result<std::monostate> write(const char *data, size_t size) override;

private:
  std::string str_;
};

// Writer that writes into `std::ostream`
class OstreamWriter : public Writer, public virtual NoCopyMove {
public:
  explicit OstreamWriter(std::ostream &out);
  static Writer::Ptr create(std::ostream &out) { return Writer::Ptr(new OstreamWriter(out)); }
  Result<std::monostate> write(const char *data, size_t size) override;

private:
  std::ostream *out_;
};

// Fast buffered writer. It works much faster than using `std::ostream` directly and is implemented
// over `Writer` interface declared above
//
// Currently, there is no error handling, and `BufWriter` panics in case of any I/O error
class BufWriter : public NoCopyMove {
public:
  // Size of the buffer
  static constexpr size_t BUFFER_SIZE = 8192U;

  explicit BufWriter(Writer *writer)
      : writer_(writer), buffer_(std::make_unique<char[]>(BUFFER_SIZE)) {}

  ~BufWriter() { flush(); }

  // Flushes the contents of the buffer to the underlying writer
  void flush();

  // Writes a string
  BufWriter &writeStr(const std::string_view &value);
  BufWriter &writeStr(const char *value) {
    return writeStr(std::string_view(value, std::strlen(value)));
  }

  // Writes an integer
  template <typename Int>
  BufWriter &writeInt(Int value);

private:
  void doWrite(const char *pos, size_t size);

  Writer *writer_;
  std::unique_ptr<char[]> buffer_;
  size_t bufPos_ = 0;
};

// Opens `std::ifstream` for reading. This is basically a wrapper over the constructor, but returns
// error in `Result` when opening failed
Result<std::ifstream, IOError> openReadFile(const char *path);
Result<std::ifstream, IOError> openReadFile(const std::string &path);

// Opens `std::ofstream` for writing. This is basically a wrapper over the constructor, but returns
// error in `Result` when opening failed
Result<std::ofstream, IOError> openWriteFile(const char *path);
Result<std::ofstream, IOError> openWriteFile(const std::string &path);

}  // namespace SoFUtil

#endif  // SOF_UTIL_IOUTIL_INCLUDED
