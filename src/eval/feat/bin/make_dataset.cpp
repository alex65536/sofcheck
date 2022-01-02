// This file is part of SoFCheck
//
// Copyright (c) 2021-2022 Alexander Kernozhitsky and SoFCheck contributors
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

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>
#include <variant>

#include "core/board.h"
#include "core/init.h"
#include "core/move.h"
#include "core/movegen.h"
#include "eval/coefs.h"
#include "eval/evaluate.h"
#include "eval/feat/feat.h"
#include "gameset/reader.h"
#include "gameset/types.h"
#include "util/fileutil.h"
#include "util/logging.h"
#include "util/misc.h"
#include "util/no_copy_move.h"
#include "util/optparse.h"
#include "util/random.h"
#include "util/result.h"

// Type of log entry
constexpr const char *MAKE_DATASET = "MakeDataset";

using namespace SoFUtil::Logging;

using SoFCore::Board;
using SoFCore::Move;
using SoFEval::coef_t;
using SoFEval::Coefs;
using SoFEval::Feat::Features;
using SoFGameSet::Game;
using SoFGameSet::Winner;
using SoFUtil::Err;
using SoFUtil::Ok;
using SoFUtil::openReadFile;
using SoFUtil::openWriteFile;
using SoFUtil::panic;
using SoFUtil::Result;

struct RichBoard {
  Winner winner;
  uint64_t gameId;
  size_t boardsTotal;
  size_t boardsLeft;
  Board board;
};

class BoardConsumer : public virtual SoFUtil::VirtualNoCopy {
public:
  virtual void consume(const RichBoard &board) = 0;
  virtual void finish() {}
};

class FeatureExtractor : public BoardConsumer {
public:
  explicit FeatureExtractor(std::ostream &out, Features features)
      : out_(&out), features_(std::move(features)) {
    writeHeader();
  }

  void consume(const RichBoard &board) override {
    const std::vector<coef_t> coefs =
        evaluator_.evalForWhite(board.board, Evaluator::Tag::from(board.board)).take();
    writeLine(board, coefs);
  }

private:
  void writeHeader() {
    auto names = features_.names();
    *out_ << "winner,game_id,board_total,board_left";
    for (const auto &name : names) {
      *out_ << "," << name.name;
    }
    *out_ << "\n";
  }

  void writeLine(const RichBoard &board, const std::vector<coef_t> &coefs) {
    *out_ << std::fixed << std::setprecision(1) << winnerToNumber(board.winner) << ","
          << board.gameId << "," << board.boardsTotal << "," << board.boardsLeft;
    for (const coef_t c : coefs) {
      *out_ << "," << c;
    }
    *out_ << "\n";
  }

  static double winnerToNumber(const Winner winner) {
    switch (winner) {
      case Winner::Black:
        return 0.0;
      case Winner::White:
        return 1.0;
      case Winner::Draw:
        return 0.5;
      case Winner::Unknown:
        panic("Winner::Unknown not supported here");
    }
    SOF_UNREACHABLE();
  }

  using Evaluator = SoFEval::Evaluator<Coefs>;

  std::ostream *out_;
  Features features_;
  Evaluator evaluator_;
};

class BoardSampler : public BoardConsumer {
public:
  BoardSampler(BoardConsumer *consumer, std::optional<uint64_t> sampleSize,
               std::optional<uint64_t> randomSeed)
      : sampleSize_(sampleSize),
        random_(randomSeed ? *randomSeed : SoFUtil::random()),
        consumer_(consumer) {}

  void consume(const RichBoard &board) override {
    if (!sampleSize_) {
      consumer_->consume(board);
      return;
    }
    ++count_;
    const uint64_t size = *sampleSize_;
    if (count_ <= size) {
      sample_.push_back(board);
      return;
    }
    if (random_() % count_ < size) {
      sample_[static_cast<size_t>(random_() % size)] = board;
    }
  }

  void finish() override {
    if (sampleSize_) {
      std::shuffle(sample_.begin(), sample_.end(), random_);
      for (const auto &board : sample_) {
        consumer_->consume(board);
      }
    }
    consumer_->finish();
  }

private:
  std::optional<uint64_t> sampleSize_;
  std::vector<RichBoard> sample_;
  std::mt19937_64 random_;
  BoardConsumer *consumer_;
  uint64_t count_ = 0;
};

class GameConsumer : public SoFUtil::NoCopy {
public:
  struct Error {
    std::string message;
  };

  explicit GameConsumer(BoardConsumer *consumer) : consumer_(consumer) {}

  Result<std::monostate, Error> consume(const Game &game, const std::vector<Board> &boards) {
    if (!game.isCanonical()) {
      return Err(Error{"The game is not canonical"});
    }
    if (game.header.winner == Winner::Unknown) {
      return Err(Error{"Games without winners are not supported"});
    }

    ++count_;

    std::vector<Move> moves;
    moves.reserve(boards.size());
    for (const auto &command : game.commands) {
      if (const auto *movesCmd = std::get_if<SoFGameSet::MovesCommand>(&command)) {
        moves.insert(moves.end(), movesCmd->moves.begin(), movesCmd->moves.end());
      }
    }

    for (size_t idx = 0; idx < boards.size(); ++idx) {
      // Ignore boards in the start of the game
      if (idx < 5) {
        continue;
      }

      // Ignore boards after capture or promote
      if (idx != 0 && (isMoveCapture(boards[idx - 1], moves[idx - 1]) ||
                       isMoveKindPromote(moves[idx - 1].kind))) {
        continue;
      }

      // Ignore boards with checks or responses to checks
      if (isCheck(boards[idx]) || (idx != 0 && isCheck(boards[idx - 1]))) {
        continue;
      }

      consumer_->consume(RichBoard{game.header.winner, count_, boards.size(),
                                   boards.size() - idx - 1, boards[idx]});
    }

    return Ok(std::monostate{});
  }

  void finish() { consumer_->finish(); }

private:
  BoardConsumer *consumer_;
  uint64_t count_ = 0;
};

struct Error {
  std::string message;
};

Result<std::monostate, Error> processGameSet(std::istream &in, GameConsumer &consumer) {
  using ReadError = SoFGameSet::GameReader::Error;
  using ReadOptions = SoFGameSet::GameReader::Options;

  SoFGameSet::GameReader reader(in, ReadOptions::CaptureBoards);
  for (;;) {
    const size_t line = reader.lineCount();
    auto readResult = reader.nextGame();
    if (readResult.isErr()) {
      consumer.finish();
      const auto error = std::move(readResult).unwrapErr();
      if (error.status == ReadError::Status::EndOfStream) {
        break;
      }
      return Err(Error{"Line " + std::to_string(error.line) + ": " + error.message});
    }
    auto consumeResult = consumer.consume(std::move(readResult).unwrap(), reader.capturedBoards());
    if (consumeResult.isErr()) {
      const auto error = std::move(consumeResult).unwrapErr();
      return Err(Error{"Line " + std::to_string(line) + ": " + error.message});
    }
  }

  return Ok(std::monostate{});
}

struct Options {
  std::optional<uint64_t> sampleSize = std::nullopt;
  std::optional<uint64_t> randomSeed = std::nullopt;
};

int run(std::istream &jsonIn, std::istream &in, std::ostream &out, const Options &options) {
  FeatureExtractor extractor(out, Features::load(jsonIn).okOrErr([](const auto err) {
    panic("Error extracting features: " + err.description);
  }));
  BoardSampler sampler(&extractor, options.sampleSize, options.randomSeed);
  GameConsumer consumer(&sampler);

  auto result = processGameSet(in, consumer);
  if (result.isErr()) {
    logFatal(MAKE_DATASET) << std::move(result).unwrapErr().message;
    return 1;
  }

  return 0;
}

constexpr const char *DESCRIPTION =
    "Extracts coefficients from the games in canonical SoFGameSet format to tune the weights in "
    "the engine";

constexpr const char *FEATURES_DESCRIPTION = "JSON file with evaluation features";
constexpr const char *INPUT_DESCRIPTION =
    "Input games in canonical SoFGameSet format. If not provided, use standard input";
constexpr const char *OUTPUT_DESCRIPTION =
    "Resulting CSV file with coefficients, which can be used then for parameter tuning. If not "
    "provided, use standard output";
constexpr const char *COUNT_DESCRIPTION =
    "Maximum number of boards to extract. If not specified, extract all the boards. The boards to "
    "extract will be selected uniformly at random";
constexpr const char *SEED_DESCRIPTION =
    "Random seed. If not specified, generate the seed randomly";

int main(int argc, char **argv) {
  std::ios_base::sync_with_stdio(false);
  SoFCore::init();

  SoFUtil::OptParser parser(argc, argv, "MakeDataset for SoFCheck");
  parser.setLongDescription(DESCRIPTION);
  parser.addOptions()                                                      //
      ("f,features", FEATURES_DESCRIPTION, cxxopts::value<std::string>())  //
      ("i,input", INPUT_DESCRIPTION, cxxopts::value<std::string>())        //
      ("o,output", OUTPUT_DESCRIPTION, cxxopts::value<std::string>())      //
      ("c,count", COUNT_DESCRIPTION, cxxopts::value<uint64_t>())           //
      ("s,seed", SEED_DESCRIPTION, cxxopts::value<uint64_t>());
  auto options = parser.parse();

  auto badFile = [&](auto err) { return panic(std::move(err.description)); };
  std::ifstream jsonIn = openReadFile(options["features"].as<std::string>()).okOrErr(badFile);
  std::ifstream fileIn;
  std::ofstream fileOut;
  std::istream *in = &std::cin;
  std::ostream *out = &std::cout;
  if (options.count("input")) {
    fileIn = openReadFile(options["input"].as<std::string>()).okOrErr(badFile);
    in = &fileIn;
  }
  if (options.count("output")) {
    fileOut = openWriteFile(options["output"].as<std::string>()).okOrErr(badFile);
    out = &fileOut;
  }

  const Options runOptions{
      options.count("count") ? std::make_optional(options["count"].as<uint64_t>()) : std::nullopt,
      options.count("seed") ? std::make_optional(options["seed"].as<uint64_t>()) : std::nullopt,
  };

  return run(jsonIn, *in, *out, runOptions);
}
