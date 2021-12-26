#ifndef SOF_UTIL_OPTPARSE_INCLUDED
#define SOF_UTIL_OPTPARSE_INCLUDED

#include <cxxopts.hpp>
#include <string>

#include "util/result.h"

namespace SoFUtil {

// Wrapper for `cxxopts::Options`. It extends the features of `cxxopts`. For example, it allows to
// use long descriptions with word-wrapping in help messages
class OptParser {
public:
  OptParser(int argc, const char *const *argv, std::string description);

  // Sets the long description. It is printed in the end of the help message
  void setLongDescription(const std::string &longDescription);

  // Adds options to the option parser. Works in the same way as `cxxopts::Options`
  cxxopts::OptionAdder addOptions(const std::string &group = "");

  // Sets the output width
  void setWidth(size_t width);

  // Returns the help message as string
  std::string help() const;

  // Parses the command line arguments. When the parser is ready to pass the arguments to the main
  // program, returns the parsed arguments. Otherwise, exits from the program
  //
  // Exiting from the program doesn't always mean that the arguments are invalid. For examples, if
  // `-h` flag is specified, the parser just shows help and exits
  cxxopts::ParseResult parse();

private:
  cxxopts::Options inner_;
  std::string description_;
  std::string longDescription_;
  int argc_;
  const char *const *argv_;
  size_t width_ = 78;
};

}  // namespace SoFUtil

#endif  // SOF_UTIL_OPTPARSE_INCLUDED
