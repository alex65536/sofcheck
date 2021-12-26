#include "util/optparse.h"

#include <cstdlib>
#include <iostream>

#include "util/strutil.h"

namespace SoFUtil {

static std::string getProgramName(const std::string &argv0) {
  for (size_t pos = argv0.size() - 1; pos != static_cast<size_t>(-1); --pos) {
    if (argv0[pos] == '/' || argv0[pos] == '\\') {
      return argv0.substr(pos + 1);
    }
  }
  return argv0;
}

OptParser::OptParser(int argc, const char *const *argv, std::string description)
    : inner_(getProgramName(argv[0])), description_(std::move(description)), argc_(argc), argv_(argv) {
  inner_.set_width(width_);
  inner_.add_options()("h,help", "Show help message");
}

void OptParser::setLongDescription(const std::string &longDescription) {
  longDescription_ = longDescription;
}

cxxopts::OptionAdder OptParser::addOptions(const std::string &group) {
  return inner_.add_options(group);
}

void OptParser::setWidth(size_t width) {
  width_ = width;
  inner_.set_width(width_);
}

std::string OptParser::help() const {
  std::string result;
  for (auto ln : wordWrap(description_, width_)) {
    result += ln;
    result += "\n";
  }
  result += inner_.help() + "\n\n";
  for (auto ln : wordWrap(longDescription_, width_)) {
    result += ln;
    result += "\n";
  }
  return result;
}

cxxopts::ParseResult OptParser::parse() {
  auto options = inner_.parse(argc_, argv_);
  if (options.count("help")) {
    std::cout << help() << std::endl;
    std::exit(0);
  }
  return options;
}

}  // namespace SoFUtil
