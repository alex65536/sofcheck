#include <json/json.h>

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "core/board.h"
#include "core/strutil.h"
#include "eval/coefs.h"
#include "eval/evaluate.h"
#include "eval/feat/feat.h"
#include "util/misc.h"
#include "util/strutil.h"

using SoFCore::Board;
using SoFEval::coef_t;
using SoFEval::Coefs;
using SoFEval::Feat::Features;
using SoFEval::Feat::LoadResult;
using SoFUtil::panic;

// Reads features from the stream, panics when encounters an error
Features readFeatures(std::istream &in) {
  Json::Value json;
  Json::CharReaderBuilder builder;
  std::string errs;
  if (!Json::parseFromStream(builder, in, &json, &errs)) {
    panic("JSON parse error: " + errs);
  }
  LoadResult<Features> maybeFeatures = Features::load(json);
  if (maybeFeatures.isErr()) {
    panic("Error extracting features: " + maybeFeatures.unwrapErr().description);
  }
  return maybeFeatures.unwrap();
}

namespace Private {

template <typename F>
F openFile(const char *name) {
  F file(name);
  if (!file.is_open()) {
    panic("Cannot open file \"" + std::string(name) + "\"");
  }
  return file;
}

}  // namespace Private

// Opens a file, panics when encounters an error
std::ifstream openInFile(const char *name) { return Private::openFile<std::ifstream>(name); }

// Opens a file, panics when encounters an error
std::ofstream openOutFile(const char *name) { return Private::openFile<std::ofstream>(name); }

class DatasetGenerator {
public:
  explicit DatasetGenerator(Features features) : features_(std::move(features)) {}

  void addLine(const double winner, const int gameId, const Board &b) {
    std::vector<coef_t> coefs = evaluator_.evaluate(b, Evaluator::Tag::from(b)).take();
    lines_.push_back({winner, gameId, std::move(coefs)});
  }

  void write(std::ostream &out) {
    writeHeader(out);
    for (const Line &line : lines_) {
      writeLine(out, line);
    }
  }

private:
  struct Line {
    double winner;
    int gameId;
    std::vector<coef_t> coefs;
  };

  void writeHeader(std::ostream &out) {
    auto names = features_.names();
    out << "winner,game_id";
    for (const auto &name : names) {
      out << "," << name.name;
    }
    out << "\n";
  }

  static void writeLine(std::ostream &out, const Line &line) {
    out << std::fixed << std::setprecision(1) << line.winner << "," << line.gameId;
    for (const coef_t c : line.coefs) {
      out << "," << c;
    }
    out << "\n";
  }

  using Evaluator = SoFEval::Evaluator<Coefs>;

  Features features_;
  std::vector<Line> lines_;
  Evaluator evaluator_;
};

int run(std::istream &jsonIn, std::istream &in, std::ostream &out) {
  DatasetGenerator gen(readFeatures(jsonIn));

  std::string s;
  size_t lineNum = 0;
  while (std::getline(in, s)) {
    ++lineNum;
    s = SoFUtil::trim(s);
    if (s.empty() || s[1] == '#') {
      // Blank and comment lines are ignored
      continue;
    }
    if (s.size() < 3) {
      std::cerr << "Line " << lineNum << ": line size must be at least 3\n";
      return 1;
    }

    double winner = 0.0;
    if (s[0] == 'B') {
      winner = 0.0;
    } else if (s[0] == 'W') {
      winner = 1.0;
    } else if (s[0] == 'D') {
      winner = 0.5;
    } else {
      std::cerr << "Line " << lineNum << ": first character must be B, W or D\n";
      return 1;
    }
    if (s[1] != ' ') {
      std::cerr << "Line " << lineNum << ": second character must be space\n";
      return 1;
    }
    s = s.substr(2);

    size_t pos = s.find(' ');
    if (pos == std::string::npos) {
      std::cerr << "Line " << lineNum << ": Space expected\n";
      return 1;
    }
    int gameId = 0;
    if (!SoFUtil::valueFromStr(s.c_str(), s.c_str() + pos, gameId)) {
      std::cerr << "Line " << lineNum << ": Game ID is not int\n";
      return 1;
    }
    s = s.substr(pos + 1);

    auto result = Board::fromFen(s.c_str());
    if (result.isErr()) {
      std::cerr << "Line " << lineNum
                << ": bad FEN: " << SoFCore::fenParseResultToStr(result.unwrapErr()) << "\n";
      return 1;
    }
    gen.addLine(winner, gameId, result.unwrap());
  }

  gen.write(out);

  return 0;
}

constexpr const char *DESCRIPTION = R"DESC(
MakeDataset for SoFCheck

This utility extracts coefficients from specified FEN boards to tune the
weights in the engine.

Usage: make_dataset FEATURES [IN_FILE [OUT_FILE]]

  FEATURES  The JSON file that contains all the evaluation features
  IN_FILE   The file with boards for which we want to create a dataset. The
            file must consist of lines "<outcomeX> <gameidX> <fenX>" where
            <outcomeX> is equal to B, W or D depending on the winning side,
            <gameidX> is equal to the ID or the game in which the position
            occurred, and <fenX> is the position description. If not specified,
            then the boards are read from the standard input
  OUT_FILE  Resulting CSV file with features, which can be used then for
            parameter tuning. If not specified, then the boards are written to
            the standard output
)DESC";

void showUsage() { std::cerr << SoFUtil::trimEolLeft(DESCRIPTION) << std::flush; }

int main(int argc, char **argv) {
  if (argc == 2 && std::strcmp(argv[1], "-h") == 0) {
    showUsage();
    return 0;
  }
  if (argc < 2 || argc > 4) {
    showUsage();
    return 1;
  }
  std::ifstream jsonIn = openInFile(argv[1]);
  std::ifstream fileIn;
  std::ofstream fileOut;
  std::istream *in = &std::cin;
  std::ostream *out = &std::cout;
  if (argc >= 3) {
    fileIn = openInFile(argv[2]);
    in = &fileIn;
  }
  if (argc >= 4) {
    fileOut = openOutFile(argv[3]);
    out = &fileOut;
  }
  return run(jsonIn, *in, *out);
}
