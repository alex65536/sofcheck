#include "common.h"

#include <iomanip>

#include "util/math.h"

using SoFUtil::log10;

void printBitboard(std::ostream &out, const SoFCore::bitboard_t val) {
  out << "0x" << std::hex << std::setw(16) << std::setfill('0') << val << std::dec
      << std::setfill(' ');
}

void SourcePrinter::arrayBody(size_t size, std::function<void(size_t)> printer) {
  stream_ << "{\n";
  indent(4);
  const size_t idxLen = (size <= 1) ? 1 : log10(size - 1) + 1;
  for (size_t i = 0; i < size; ++i) {
    lineStart() << "/*" << std::setw(idxLen) << i << "*/ ";
    printer(i);
    if (i + 1 != size) {
      stream_ << ",";
    }
    stream_ << "\n";
  }
  outdent(4);
  line() << "};";
}

void SourcePrinter::bitboardArray(const char *name, const std::vector<SoFCore::bitboard_t> &array) {
  lineStart() << "constexpr bitboard_t " << name << "[" << array.size() << "] = ";
  arrayBody(array.size(), [&](const size_t idx) { printBitboard(stream_, array[idx]); });
}

void SourcePrinter::coordArray(const char *name, const std::vector<SoFCore::coord_t> &array) {
  lineStart() << "constexpr coord_t " << name << "[" << array.size() << "] = ";
  arrayBody(array.size(), [&](const size_t idx) { stream_ << static_cast<int>(array[idx]); });
}

void SourcePrinter::startHeaderGuard(const char *name) {
  headerGuardName_ = name;
  line() << "#ifndef " << name;
  line() << "#define " << name;
}

void SourcePrinter::endHeaderGuard() { line() << "#endif  // " << headerGuardName_; }

SourcePrinter::Line::Line(SourcePrinter &printer, bool printEoln)
    : printer_(printer), printEoln_(printEoln) {
  printer.stream_ << std::setw(printer.indent_) << std::setfill(' ') << "";
}

SourcePrinter::Line::~Line() {
  if (printEoln_) {
    printer_.stream() << "\n";
  }
}
