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

void Logger::log(LogLevel level, const char *type, const std::string &message) {
  std::string str = std::string(logLevelToStr(level)) + " [" + type + "]: " + message + "\n";
  std::cerr << str << std::flush;
}

}  // namespace SoFUtil
