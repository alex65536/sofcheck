// This file is part of SoFCheck
//
// Copyright (c) 2020 Alexander Kernozhitsky and SoFCheck contributors
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

[[noreturn]] void Private::assertFail(const char *file, const int line, const char *msg) {
  panic(std::string("Assertion failed: ") + file + ":" + std::to_string(line) + ": " + msg);
}

}  // namespace SoFUtil
