#ifndef SOF_GEN_COMMON_INCLUDED
#define SOF_GEN_COMMON_INCLUDED

#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "core/types.h"
#include "util/no_copy_move.h"

class SourcePrinter {
public:
  explicit SourcePrinter(std::ostream &stream) : stream_(stream) {}

  class Line : public SoFUtil::NoCopyMove {
  public:
    ~Line();

    template <typename T>
    Line &operator<<(const T &other) {
      printer_.stream_ << other;
      return *this;
    }

  private:
    friend class SourcePrinter;

    Line(SourcePrinter &printer, bool printEoln);

    SourcePrinter &printer_;
    bool printEoln_;
  };

  friend class Line;

  void skip() { stream_ << "\n"; }

  Line line() { return Line(*this, true); }
  Line lineStart() { return Line(*this, false); }

  std::ostream &stream() { return stream_; }

  void indent(const size_t amount) { indent_ += amount; }
  void outdent(const size_t amount) { indent_ -= amount; }

  void arrayBody(size_t size, std::function<void(size_t)> printer);

  template <typename T>
  void array(const char *name, const char *signature, const std::vector<T> &array) {
    lineStart() << "constexpr " << signature << " " << name << "[" << array.size() << "] = ";
    arrayBody(array.size(), [&](const size_t idx) { stream_ << array[idx]; });
  }

  void bitboardArray(const char *name, const std::vector<SoFCore::bitboard_t> &array);
  void coordArray(const char *name, const std::vector<SoFCore::coord_t> &array);

  void startHeaderGuard(const char *name);
  void endHeaderGuard();

private:
  std::ostream &stream_;
  std::string headerGuardName_;
  size_t indent_ = 0;
};

#endif  // SOF_GEN_COMMON_INCLUDED
