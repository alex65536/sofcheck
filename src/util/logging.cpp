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

#include "util/logging.h"

#include <iostream>

namespace SoFUtil {

const char *logLevelToStr(LogLevel level) {
  switch (level) {
    case LogLevel::Debug:
      return "Debug";
    case LogLevel::Info:
      return "Info";
    case LogLevel::Warn:
      return "Warning";
    case LogLevel::Error:
      return "Error";
    case LogLevel::Fatal:
      return "Fatal";
  }
  return "";
}

Logger &logger() {
  static Logger instance;
  return instance;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void Logger::log(LogLevel level, const char *type, const std::string &message) {
  std::string str = std::string(logLevelToStr(level)) + " [" + type + "]: " + message + "\n";
  std::cerr << str << std::flush;
}

}  // namespace SoFUtil
