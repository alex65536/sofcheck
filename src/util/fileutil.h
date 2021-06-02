#ifndef SOF_UTIL_FILEUTIL_INCLUDED
#define SOF_UTIL_FILEUTIL_INCLUDED

#include <fstream>
#include <string>

#include "util/result.h"

namespace SoFUtil {

struct FileError {
  std::string description;
};

// Opens `std::ifstream` for reading. This is basically a wrapper over the constructor, but returns
// error in `Result` when opening failed
Result<std::ifstream, FileError> openReadFile(const char *path);

// Opens `std::ofstream` for writing. This is basically a wrapper over the constructor, but returns
// error in `Result` when opening failed
Result<std::ofstream, FileError> openWriteFile(const char *path);

}  // namespace SoFUtil

#endif  // SOF_UTIL_FILEUTIL_INCLUDED
