#include "util/misc.h"

#include <exception>
#include <iostream>

namespace SoFUtil {

[[noreturn]] void panic(const char *message) {
  std::cerr << "Program panicked: " << message << std::endl;
  std::terminate();
}

}  // namespace SoFUtil
