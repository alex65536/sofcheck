#include "util/misc.h"

#include <exception>
#include <iostream>

#include "config.h"

#ifdef USE_BOOST_STACKTRACE
#include <boost/stacktrace/stacktrace.hpp>
#endif

namespace SoFUtil {

[[noreturn]] void panic(const char *message) {
  std::cerr << "Program panicked: " << message << std::endl;
#ifdef USE_BOOST_STACKTRACE
  std::cerr << "Stack trace: " << std::endl << boost::stacktrace::stacktrace() << std::flush;
#endif
  std::terminate();
}

[[noreturn]] void panic(const std::string &message) { panic(message.c_str()); }

}  // namespace SoFUtil
