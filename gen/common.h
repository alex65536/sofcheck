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
    stream_ << ";\n";
  }

  void bitboardArray(const char *name, const std::vector<SoFCore::bitboard_t> &array);
  void coordArray(const char *name, const std::vector<SoFCore::coord_t> &array);

  void headerGuard(const std::string &name);

  class NamespaceScope : public SoFUtil::NoCopyMove {
  public:
    ~NamespaceScope();

  private:
    friend class SourcePrinter;

    NamespaceScope(SourcePrinter &printer, std::string name);

    SourcePrinter &printer_;
    std::string name_;
  };

  NamespaceScope inNamespace(const std::string &name) { return NamespaceScope(*this, name); }
  void include(const char *header);
  void sysInclude(const char *header);

  ~SourcePrinter();

private:
  friend class Line;

  std::ostream &stream_;
  std::string headerGuardName_;
  bool hasHeaderGuard_ = false;
  size_t indent_ = 0;
};

#endif  // SOF_GEN_COMMON_INCLUDED
