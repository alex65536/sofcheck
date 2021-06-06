#include "util/formatter.h"

#include <iomanip>
#include <utility>

namespace SoFUtil {

SourceFormatter::Line::Line(SourceFormatter &fmt, const bool printEoln)
    : fmt_(fmt), printEoln_(printEoln) {
  fmt.stream_ << std::setw(fmt.indent_) << std::setfill(' ') << "";
}

SourceFormatter::Line::~Line() {
  if (printEoln_) {
    fmt_.stream() << "\n";
  }
}

}  // namespace SoFUtil
