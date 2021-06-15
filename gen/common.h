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

#ifndef SOF_GEN_COMMON_INCLUDED
#define SOF_GEN_COMMON_INCLUDED

#include <cstddef>
#include <functional>
#include <ostream>
#include <string>
#include <vector>

#include "core/types.h"
#include "util/formatter.h"
#include "util/no_copy_move.h"

class SourcePrinter {
public:
  explicit SourcePrinter(std::ostream &stream);

  using Line = SoFUtil::SourceFormatter::Line;

  void skip() { inner_.skip(); }
  Line line() { return inner_.line(); }
  Line lineStart() { return inner_.lineStart(); }
  std::ostream &stream() { return inner_.stream(); }
  void indent(const size_t amount) { inner_.indent(amount); }
  void outdent(const size_t amount) { inner_.outdent(amount); }

  void arrayBody(size_t size, const std::function<void(size_t)> &printer);

  template <typename T>
  void array(const char *name, const char *signature, const std::vector<T> &array) {
    lineStart() << "constexpr " << signature << " " << name << "[" << array.size() << "] = ";
    arrayBody(array.size(), [&](const size_t idx) { stream() << array[idx]; });
    stream() << ";\n";
  }

  void bitboardArray(const char *name, const std::vector<SoFCore::bitboard_t> &array);
  void coordArray(const char *name, const std::vector<SoFCore::coord_t> &array);

  void headerGuard(const std::string &name);

  class NamespaceScope : public SoFUtil::NoCopyMove {
  public:
    ~NamespaceScope();

  private:
    friend class SourcePrinter;

    NamespaceScope(SourcePrinter &printer, std::string name);

    SourcePrinter &printer_;
    std::string name_;
  };

  NamespaceScope inNamespace(const std::string &name) { return NamespaceScope(*this, name); }
  void include(const char *header);
  void sysInclude(const char *header);

  ~SourcePrinter();

private:
  SoFUtil::SourceFormatter inner_;
  std::string headerGuardName_;
  bool hasHeaderGuard_ = false;
};

#endif  // SOF_GEN_COMMON_INCLUDED
