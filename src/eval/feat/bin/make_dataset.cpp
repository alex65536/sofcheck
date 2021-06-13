// This file is part of SoFCheck
//
// Copyright (c) 2021 Alexander Kernozhitsky and SoFCheck contributors
//
// SoFCheck is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SoFCheck is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

#include <json/json.h>

#include <array>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

#include "core/board.h"
#include "core/init.h"
#include "core/movegen.h"
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

enum class Winner { White, Black, Draw };

inline constexpr std::optional<Winner> winnerFromChar(const char ch) {
  switch (ch) {
    case 'W':
      return Winner::White;
    case 'B':
      return Winner::Black;
    case 'D':
      return Winner::Draw;
    default:
      return std::nullopt;
  }
  SOF_UNREACHABLE();
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
    std::vector<bool> isBoardGood(game.boards.size(), true);

    auto getMaterialEstimate = [&](const Board &b) {
      std::array<size_t, Board::BB_PIECES_SZ> counts{};
      for (const SoFCore::cell_t cell : b.cells) {
        ++counts[cell];
      }
      return counts;
    };

    // Find the positions where tactics play an important role in the evaluation result. Such
    // positions include boards after captures, checks and responses to checks. Evaluation doesn't
    // give a good estimate of the position cost on such boards, so don't include it to the dataset
    for (size_t idx = 0; idx < game.boards.size(); ++idx) {
      // Ignore moves in the start of the game
      if (idx < 5) {
        isBoardGood[idx] = false;
      }

      // Captures
      if (idx != 0 &&
          getMaterialEstimate(game.boards[idx - 1]) != getMaterialEstimate(game.boards[idx])) {
        isBoardGood[idx] = false;
      }

      // Checks
      if (idx != 0 && isCheck(game.boards[idx - 1])) {
        isBoardGood[idx - 1] = false;
        isBoardGood[idx] = false;
      }
    }

    // Evaluate good positions and add the coefficients from them into `lines_`
    for (size_t idx = 0; idx < game.boards.size(); ++idx) {
      if (!isBoardGood[idx]) {
        continue;
      }
      const Board &b = game.boards[idx];
      std::vector<coef_t> coefs = evaluator_.evalForWhite(b, Evaluator::Tag::from(b)).take();
      lines_.push_back({winnerToNumber(game.winner), game.id, game.boards.size(),
                        game.boards.size() - 1 - idx, std::move(coefs)});
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
    size_t boardsTotal;
    size_t boardsLeft;
    std::vector<coef_t> coefs;
  };

  void writeHeader(std::ostream &out) {
    auto names = features_.names();
    out << "winner,game_id,board_total,board_left";
    for (const auto &name : names) {
      out << "," << name.name;
    }
    out << "\n";
  }

  static void writeLine(std::ostream &out, const Line &line) {
    out << std::fixed << std::setprecision(1) << line.winner << "," << line.gameId << ","
        << line.boardsTotal << "," << line.boardsLeft;
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
    enum class Kind { Error, EndOfStream };
    Kind kind;
    size_t line;
    std::string description;

    static Status error(const size_t line, std::string description) {
      return Status{Kind::Error, line, std::move(description)};
    }

    static Status endOfStream() { return Status{Kind::EndOfStream, 0, ""}; }
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
      return SoFUtil::Err(Status::endOfStream());
    }
    const auto tokens = SoFUtil::split(ln->c_str());
    if (tokens.size() != 3) {
      return SoFUtil::Err(error("Game header must contain exactly three fields"));
    }

    if (tokens[0] != "game") {
      return SoFUtil::Err(error("Invalid game header"));
    }

    if (tokens[1].size() != 1) {
      return SoFUtil::Err(error("Winner must be single character"));
    }
    std::optional<Winner> winner = winnerFromChar(tokens[1].front());
    if (!winner) {
      return SoFUtil::Err(error("Invalid winner character"));
    }

    size_t id = 0;
    if (!SoFUtil::valueFromStr(tokens[2], id)) {
      return SoFUtil::Err(error("Invalid game ID"));
    }

    return SoFUtil::Ok(Game(*winner, id));
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
      std::string line(SoFUtil::trim(*maybeLine));
      if (line.empty() || line[0] == '#') {
        continue;
      }
      lastLine_ = std::make_optional(std::move(line));
      break;
    }
    return lastLine_;
  }

  std::optional<std::string> peekLine() const { return lastLine_; }

  size_t line_ = 0;
  std::optional<std::string> lastLine_ = std::nullopt;
  std::istream &in_;
};

int run(std::istream &jsonIn, std::istream &in, std::ostream &out) {
  DatasetGenerator gen(Features::load(jsonIn).okOrErr(
      [](const auto err) { panic("Error extracting features: " + err.description); }));
  GameParser parser(in);

  auto addGames = [&]() -> Result<std::monostate, GameParser::Status> {
    for (;;) {
      // Load games one by one until we get an error or encounter the end of the stream
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
  IN_FILE   The file with boards from which we want to extract coefficients.
            The format of this file is described below. If this argument is
            not provided, then the boards are read from the standard input
  OUT_FILE  Resulting CSV file with coefficients, which can be used then for
            parameter tuning. If this argument is not provided, then the
            boards are written to the standard output

The file with boards has the following format. It contains one or more game
sections. Each game section starts with line "game <winner> <id>", where
<winner> is equal to B, W or D depending on the winning side, and <id> is
equal to some unsigned integer called the game ID. Each of the following lines
describe a single board and has the format "board <fen>", where <fen> is the
board encoded as FEN. The boards arrive in the same order as they were played.
It's also worth to mention that the blank lines and the lines starting with
"#" are ignored in the file.
)DESC";

void showUsage() { std::cerr << SoFUtil::trimEolLeft(DESCRIPTION) << std::flush; }

int main(int argc, char **argv) {
  std::ios_base::sync_with_stdio(false);
  SoFCore::init();

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
