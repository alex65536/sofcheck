#ifndef SOF_UTIL_LOGGING_INCLUDED
#define SOF_UTIL_LOGGING_INCLUDED

#include <sstream>

#include "util/operators.h"

namespace SoFUtil {

// Type of the log entry
enum LogLevel { Debug = 0, Info = 1, Warn = 2, Error = 3, Fatal = 4 };

SOF_ENUM_COMPARE(LogLevel, int)

// Converts `LogLevel` to string
const char *logLevelToStr(LogLevel level);

// Log writer. This class is a singleton and is accessible only via `logger()` function. All the
// methods in this class are thread-safe.
class Logger {
public:
  // Adds a log entry. `level` denotes the importance of the entry, `type` denotes where the log
  // entry came from, and `message` is the message to be written into logs.
  void log(LogLevel level, const char *type, const std::string &message);

  // Convenience methods to add a log entry of a specific level
  inline void debug(const char *type, const std::string &message) {
    log(LogLevel::Debug, type, message);
  }

  inline void info(const char *type, const std::string &message) {
    log(LogLevel::Info, type, message);
  }

  inline void warn(const char *type, const std::string &message) {
    log(LogLevel::Warn, type, message);
  }

  inline void error(const char *type, const std::string &message) {
    log(LogLevel::Error, type, message);
  }

  inline void fatal(const char *type, const std::string &message) {
    log(LogLevel::Fatal, type, message);
  }

private:
  friend Logger &logger();

  Logger() = default;
};

Logger &logger();

// Convenience class to make logging simpler. It builds the log entry with `<<` operator and adds it
// on destruction.
class LogEntryStream {
public:
  inline LogEntryStream(LogLevel level, const char *type) : level_(level), type_(type) {}
  inline ~LogEntryStream() { logger().log(level_, type_, stream_.str()); }

  template <typename T>
  LogEntryStream &operator<<(const T &item) {
    stream_ << item;
    return *this;
  }

private:
  LogLevel level_;
  const char *type_;
  std::ostringstream stream_;
};

// Convenience functions. The usage pattern looks like this:
//
// logDebug("My type") << "Debug info: " << info;
//
// You can add `using namespace SoFUtil::Logging;` into your code in order to use these functions
namespace Logging {
inline LogEntryStream logDebug(const char *type) { return LogEntryStream(LogLevel::Debug, type); }
inline LogEntryStream logInfo(const char *type) { return LogEntryStream(LogLevel::Info, type); }
inline LogEntryStream logWarn(const char *type) { return LogEntryStream(LogLevel::Warn, type); }
inline LogEntryStream logError(const char *type) { return LogEntryStream(LogLevel::Error, type); }
inline LogEntryStream logFatal(const char *type) { return LogEntryStream(LogLevel::Fatal, type); }
}  // namespace Logging

}  // namespace SoFUtil

#endif  // SOF_UTIL_LOGGING_INCLUDED
