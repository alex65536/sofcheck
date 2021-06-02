#include "util/fileutil.h"

namespace SoFUtil {

template <typename F>
inline static Result<F, FileError> openFileCommon(const char *path) {
  F file(path);
  if (!file.is_open()) {
    return Err(FileError{std::string("Unable to open file \"") + path + "\""});
  }
  return Ok(std::move(file));
}

Result<std::ifstream, FileError> openReadFile(const char *path) {
  return openFileCommon<std::ifstream>(path);
}

Result<std::ofstream, FileError> openWriteFile(const char *path) {
  return openFileCommon<std::ofstream>(path);
}

}  // namespace SoFUtil
