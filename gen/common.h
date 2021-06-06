#ifndef SOF_GEN_COMMON_INCLUDED
#define SOF_GEN_COMMON_INCLUDED

#include <functional>
#include <ostream>
#include <string>
#include <vector>

#include "core/types.h"
#include "util/formatter.h"
#include "util/no_copy_move.h"

class SourcePrinter {
public:
  explicit SourcePrinter(std::ostream &stream) : inner_(stream, 2) {}

  using Line = SoFUtil::SourceFormatter::Line;

  void skip() { inner_.skip(); }
  Line line() { return inner_.line(); }
  Line lineStart() { return inner_.lineStart(); }
  std::ostream &stream() { return inner_.stream(); }
  void indent(const size_t amount) { inner_.indent(amount); }
  void outdent(const size_t amount) { inner_.outdent(amount); }

  void arrayBody(size_t size, const std::function<void(size_t)> &printer);

  template <typename T>
  void array(const char *name, const char *signature, const std::vector<T> &array) {
    lineStart() << "constexpr " << signature << " " << name << "[" << array.size() << "] = ";
    arrayBody(array.size(), [&](const size_t idx) { stream() << array[idx]; });
    stream() << ";\n";
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
  SoFUtil::SourceFormatter inner_;
  std::string headerGuardName_;
  bool hasHeaderGuard_ = false;
};

void printBitboard(std::ostream &out, SoFCore::bitboard_t val);

#endif  // SOF_GEN_COMMON_INCLUDED
