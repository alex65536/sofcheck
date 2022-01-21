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

#include "common.h"

#include <iomanip>
#include <utility>

#include "util/misc.h"
#include "util/strutil.h"

void printBitboard(std::ostream &out, const SoFCore::bitboard_t val) {
  out << "0x" << std::hex << std::setw(16) << std::setfill('0') << val << std::dec
      << std::setfill(' ');
}

SourcePrinter::SourcePrinter(std::ostream &stream) : inner_(stream, 2) {
  line() << "// This file is generated automatically, DO NOT EDIT!";
  skip();
}

void SourcePrinter::arrayBody(size_t size, const std::function<void(size_t)> &printer) {
  stream() << "{\n";
  indent(2);
  const size_t idxLen = (size == 0) ? 1 : SoFUtil::uintStrLen(size - 1);
  for (size_t i = 0; i < size; ++i) {
    lineStart() << "/*" << std::setw(idxLen) << i << "*/ ";
    printer(i);
    if (i + 1 != size) {
      stream() << ",";
    }
    stream() << "\n";
  }
  outdent(2);
  lineStart() << "}";
}

void SourcePrinter::bitboardArray(const char *name, const std::vector<SoFCore::bitboard_t> &array) {
  lineStart() << "constexpr SoFCore::bitboard_t " << name << "[" << array.size() << "] = ";
  arrayBody(array.size(), [&](const size_t idx) { printBitboard(stream(), array[idx]); });
  stream() << ";\n";
}

void SourcePrinter::coordArray(const char *name, const std::vector<SoFCore::coord_t> &array) {
  lineStart() << "constexpr SoFCore::coord_t " << name << "[" << array.size() << "] = ";
  arrayBody(array.size(), [&](const size_t idx) { stream() << static_cast<int>(array[idx]); });
  stream() << ";\n";
}

void SourcePrinter::headerGuard(const std::string &name) {
  headerGuardName_ = name;
  hasHeaderGuard_ = true;
  line() << "#ifndef " << name;
  line() << "#define " << name;
}

void SourcePrinter::skipHeaderGuard() { skipHeaderGuard_ = true; }

void SourcePrinter::include(const char *header) { line() << "#include \"" << header << "\""; }

void SourcePrinter::sysInclude(const char *header) { line() << "#include <" << header << ">"; }

SourcePrinter::NamespaceScope::NamespaceScope(SourcePrinter &printer, std::string name)
    : printer_(printer), name_(std::move(name)) {
  printer_.line() << "namespace " << name_ << " {";
}

SourcePrinter::NamespaceScope::~NamespaceScope() { printer_.line() << "}  // namespace " << name_; }

SourcePrinter::~SourcePrinter() {
  if (!skipHeaderGuard_) {
    SOF_ASSERT_MSG("No header guard was specified", hasHeaderGuard_);
    skip();
    line() << "#endif  // " << headerGuardName_;
  }
}
