#include "common.h"

#include <iomanip>
#include <utility>

#include "util/math.h"
#include "util/misc.h"

using SoFUtil::log10;

void printBitboard(std::ostream &out, const SoFCore::bitboard_t val) {
  out << "0x" << std::hex << std::setw(16) << std::setfill('0') << val << std::dec
      << std::setfill(' ');
}

void SourcePrinter::arrayBody(size_t size, const std::function<void(size_t)> &printer) {
  stream() << "{\n";
  indent(2);
  const size_t idxLen = (size <= 1) ? 1 : log10(size - 1) + 1;
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
  lineStart() << "constexpr bitboard_t " << name << "[" << array.size() << "] = ";
  arrayBody(array.size(), [&](const size_t idx) { printBitboard(stream(), array[idx]); });
  stream() << ";\n";
}

void SourcePrinter::coordArray(const char *name, const std::vector<SoFCore::coord_t> &array) {
  lineStart() << "constexpr coord_t " << name << "[" << array.size() << "] = ";
  arrayBody(array.size(), [&](const size_t idx) { stream() << static_cast<int>(array[idx]); });
  stream() << ";\n";
}

void SourcePrinter::headerGuard(const std::string &name) {
  headerGuardName_ = name;
  hasHeaderGuard_ = true;
  line() << "#ifndef " << name;
  line() << "#define " << name;
}

void SourcePrinter::include(const char *header) { line() << "#include \"" << header << "\""; }

void SourcePrinter::sysInclude(const char *header) { line() << "#include <" << header << ">"; }

SourcePrinter::NamespaceScope::NamespaceScope(SourcePrinter &printer, std::string name)
    : printer_(printer), name_(std::move(name)) {
  printer_.line() << "namespace " << name_ << " {";
}

SourcePrinter::NamespaceScope::~NamespaceScope() { printer_.line() << "}  // namespace " << name_; }

SourcePrinter::~SourcePrinter() {
  SOF_ASSERT_MSG("No header guard was specified", hasHeaderGuard_);
  skip();
  line() << "#endif  // " << headerGuardName_;
}
