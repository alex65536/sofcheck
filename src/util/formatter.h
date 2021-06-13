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

#ifndef SOF_UTIL_FORMATTER_INCLUDED
#define SOF_UTIL_FORMATTER_INCLUDED

#include <ostream>

#include "util/no_copy_move.h"

namespace SoFUtil {

// Wrapper over `std::ostream` to generate source code programmatically in a more convenient way. It
// supports automatic line endings and indentation
class SourceFormatter {
public:
  // Creates `SourceFormater`. `indentStep` is the amount of spaces for one indentation unit
  explicit SourceFormatter(std::ostream &stream, const size_t indentStep)
      : stream_(stream), indentStep_(indentStep) {}

  // Helper RAII wrapper to print a line to the formatter
  class Line : public SoFUtil::NoCopyMove {
  public:
    ~Line();

    template <typename T>
    Line &operator<<(const T &other) {
      fmt_.stream_ << other;
      return *this;
    }

  private:
    friend class SourceFormatter;

    Line(SourceFormatter &fmt, bool printEoln);

    SourceFormatter &fmt_;
    bool printEoln_;
  };

  // Just prints an empty line
  void skip() { stream_ << "\n"; }

  // Helper functions to print the line. When the line printing is started, the indentation is
  // added. `line()` returns an object that puts line ending when destroyed, while the object
  // returned by `lineStart()` does nothing on its destruction. The usage is as follows:
  //
  // line() << 1 << " " << 2 << " " << 3;  /* Prints "1 2 3\n" with indentation */
  // lineStart() << "Hello!";  /* Prints "Hello!" with indentation */
  Line line() { return Line(*this, true); }
  Line lineStart() { return Line(*this, false); }

  // Returns the underlying stream
  std::ostream &stream() { return stream_; }

  // These functions control the indentation. `indent()` increases the indentation by `amount`
  // units, `outdent()` decreases the indentation by `amount` units
  void indent(const size_t amount) { indent_ += amount * indentStep_; }
  void outdent(const size_t amount) { indent_ -= amount * indentStep_; }

private:
  friend class Line;

  std::ostream &stream_;
  size_t indentStep_;
  size_t indent_ = 0;
};

}  // namespace SoFUtil

#endif  // SOF_UTIL_FORMATTER_INCLUDED
