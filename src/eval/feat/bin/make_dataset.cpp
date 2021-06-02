#include <json/json.h>

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

#include "core/board.h"
#include "core/strutil.h"
#include "eval/coefs.h"
#include "eval/evaluate.h"
#include "eval/feat/feat.h"
#include "util/fileutil.h"
#include "util/logging.h"
#include "util/misc.h"
#include "util/result.h"
#include "util/strutil.h"

// Type of log entry
constexpr const char *MAKE_DATASET = "MakeDataset";

using namespace SoFUtil::Logging;

using SoFCore::Board;
using SoFEval::coef_t;
using SoFEval::Coefs;
using SoFEval::Feat::Features;
using SoFUtil::openReadFile;
using SoFUtil::openWriteFile;
using SoFUtil::panic;
using SoFUtil::Result;

// Reads features from the stream, panics when encounters an error
Features readFeatures(std::istream &in) {
  Json::Value json;
  Json::CharReaderBuilder builder;
  std::string errs;
  if (!Json::parseFromStream(builder, in, &json, &errs)) {
    panic("JSON parse error: " + errs);
  }
  return Features::load(json).okOrErr(
      [](const auto err) { panic("Error extracting features: " + err.description); });
}

enum class Winner { White, Black, Draw };

inline constexpr std::optional<Winner> winnerFromChar(const char ch) {
  if (ch == 'W') {
    return Winner::White;
  }
  if (ch == 'B') {
    return Winner::Black;
  }
  if (ch == 'D') {
    return Winner::Draw;
  }
  return std::nullopt;
}

struct Game {
  Winner winner;
  size_t id;
  std::vector<Board> boards;

  Game() = default;

  Game(const Winner winner, const size_t id) : winner(winner), id(id) {}

  Game &add(const Board &board) {
    boards.push_back(board);
    return *this;
  }
};

class DatasetGenerator {
public:
  explicit DatasetGenerator(Features features) : features_(std::move(features)) {}

  void addGame(const Game &game) {
    for (const Board &b : game.boards) {
      std::vector<coef_t> coefs = evaluator_.evaluate(b, Evaluator::Tag::from(b)).take();
      lines_.push_back({winnerToNumber(game.winner), game.id, std::move(coefs)});
    }
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
    size_t gameId;
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

  static double winnerToNumber(const Winner winner) {
    switch (winner) {
      case Winner::Black:
        return 0.0;
      case Winner::White:
        return 1.0;
      case Winner::Draw:
        return 0.5;
    }
    SOF_UNREACHABLE();
  }

  using Evaluator = SoFEval::Evaluator<Coefs>;

  Features features_;
  std::vector<Line> lines_;
  Evaluator evaluator_;
};

class GameParser {
public:
  struct Status {
    enum class Kind { Error, EndOfFile };
    Kind kind;
    size_t line;
    std::string description;

    static Status error(const size_t line, std::string description) {
      return Status{Kind::Error, line, std::move(description)};
    }

    static Status endOfFile() { return Status{Kind::EndOfFile, 0, ""}; }
  };

  explicit GameParser(std::istream &in) : in_(in) { readLine(); }

  Result<Game, Status> nextGame() {
    SOF_TRY_DECL(game, readGameHeader());
    for (;;) {
      auto ln = readLine();
      if (!ln || !SoFUtil::startsWith(*ln, "board ")) {
        break;
      }
      const char *fen = ln->c_str() + std::strlen("board ");
      SOF_TRY_DECL(board, Board::fromFen(fen).mapErr([this](const auto err) {
        return error(std::string("FEN parse error: ") + SoFCore::fenParseResultToStr(err));
      }));
      game.boards.push_back(board);
    }
    return SoFUtil::Ok(std::move(game));
  }

private:
  Status error(const std::string &description) const { return Status::error(line_, description); }

  Result<Game, Status> readGameHeader() {
    auto ln = peekLine();
    if (!ln) {
      return SoFUtil::Err(Status::endOfFile());
    }
    const char *pos = SoFUtil::scanTokenStart(ln->c_str());

    // "game" word
    if (*pos != '\0') {
      const char *nxt = SoFUtil::scanTokenEnd(pos);
      if (std::string(pos, nxt) != "game") {
        return SoFUtil::Err(error("Invalid game header"));
      }
      pos = SoFUtil::scanTokenStart(nxt);
    } else {
      return SoFUtil::Err(error("Game header too short"));
    }

    // Winner field
    Winner winner = Winner::White;
    if (*pos != '\0') {
      const char *nxt = SoFUtil::scanTokenEnd(pos);
      if (nxt != pos + 1) {
        return SoFUtil::Err(error("Winner must be single character"));
      }
      if (auto maybeWinner = winnerFromChar(*pos); maybeWinner.has_value()) {
        winner = *maybeWinner;
      } else {
        return SoFUtil::Err(error("Invalid winner character"));
      }
      pos = SoFUtil::scanTokenStart(nxt);
    } else {
      return SoFUtil::Err(error("Game header too short"));
    }

    // Game ID
    size_t id = 0;
    if (*pos != '\0') {
      const char *nxt = SoFUtil::scanTokenEnd(pos);
      if (!SoFUtil::valueFromStr(pos, nxt, id)) {
        return SoFUtil::Err(error("Invalid game ID"));
      }
      pos = SoFUtil::scanTokenStart(nxt);
    } else {
      return SoFUtil::Err(error("Game header too short"));
    }

    if (*pos != '\0') {
      return SoFUtil::Err(error("Extra data in the line"));
    }
    return SoFUtil::Ok(Game(winner, id));
  }

  std::optional<std::string> readLineRaw() {
    std::string result;
    if (std::getline(in_, result)) {
      ++line_;
      return result;
    }
    return std::nullopt;
  }

  std::optional<std::string> readLine() {
    for (;;) {
      auto maybeLine = readLineRaw();
      if (!maybeLine) {
        lastLine_ = std::nullopt;
        break;
      }
      std::string line = SoFUtil::trim(*maybeLine);
      if (line.empty() || line[0] == '#') {
        continue;
      }
      lastLine_ = line;
      break;
    }
    return lastLine_;
  }

  std::optional<std::string> peekLine() { return lastLine_; }

  size_t line_ = 0;
  std::optional<std::string> lastLine_ = std::nullopt;
  std::istream &in_;
};

int run(std::istream &jsonIn, std::istream &in, std::ostream &out) {
  DatasetGenerator gen(readFeatures(jsonIn));
  GameParser parser(in);

  auto addGames = [&]() -> Result<std::monostate, GameParser::Status> {
    for (;;) {
      SOF_TRY_DECL(game, parser.nextGame());
      gen.addGame(game);
    }
  };

  const auto status = addGames().unwrapErr();
  if (status.kind == GameParser::Status::Kind::Error) {
    logFatal(MAKE_DATASET) << "Line " << status.line << ": " << status.description;
    return 1;
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
            format of this file is described below. If not specified, then the
            boards are read from the standard input
  OUT_FILE  Resulting CSV file with features, which can be used then for
            parameter tuning. If not specified, then the boards are written to
            the standard output

The file with boards must contain one or more game sections. Each game section
starts with line "game <winner> <id>", where <winner> is equal to B, W or D
depending on the winning side, and <id> is equal to some unsigned integer
called the game ID. Each of the following lines describes a single board and
has the format "board <fen>", where <fen> is the board encoded as FEN. The
boards arrive in the same order as they were played. It's also worth to
mention that the blank lines and the lines starting with # are ignored in the
file.
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
  auto badFile = [&](auto err) { return panic(std::move(err.description)); };
  std::ifstream jsonIn = openReadFile(argv[1]).okOrErr(badFile);
  std::ifstream fileIn;
  std::ofstream fileOut;
  std::istream *in = &std::cin;
  std::ostream *out = &std::cout;
  if (argc >= 3) {
    fileIn = openReadFile(argv[2]).okOrErr(badFile);
    in = &fileIn;
  }
  if (argc >= 4) {
    fileOut = openWriteFile(argv[3]).okOrErr(badFile);
    out = &fileOut;
  }
  return run(jsonIn, *in, *out);
}
