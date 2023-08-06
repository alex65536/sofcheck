// This file is part of SoFCheck
//
// Copyright (c) 2022 Alexander Kernozhitsky and SoFCheck contributors
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

#include "gameset/reader.h"

#include <istream>
#include <string_view>
#include <type_traits>

#include "core/move_parser.h"
#include "core/movegen.h"
#include "core/strutil.h"
#include "util/misc.h"
#include "util/strutil.h"

namespace SoFGameSet {

using SoFCore::Board;
using SoFCore::Move;
using SoFUtil::Err;
using SoFUtil::makeCopyPtr;
using SoFUtil::Ok;
using SoFUtil::Result;

GameReader::GameReader(std::istream &in, Options options) : in_(&in), options_(options) {
  lastCommand_ = readCommand();
}

Result<Game, GameReader::Error> GameReader::nextGame() {
  SOF_TRY_DECL(firstCommand, lastCommand_);
  auto *gameCommand = std::get_if<GameCommand>(&firstCommand);
  if (!gameCommand) {
    return error("First command must be \"game\"");
  }
  Game game(std::move(*gameCommand));
  capturedBoards_.clear();

  for (;;) {
    auto readResult = readCommand();
    if (readResult.isErr()) {
      lastCommand_ = readResult;
      auto error = std::move(readResult).unwrapErr();
      if (error.status == Error::Status::EndOfStream) {
        return Ok(std::move(game));
      }
      return Err(std::move(error));
    }
    const bool breakFromLoop = std::visit(
        [&](auto command) {
          using CommandType = std::decay_t<decltype(command)>;
          if constexpr (std::is_same_v<CommandType, GameCommand>) {
            lastCommand_ = Ok<AnyCommand>(std::move(command));
            return true;
          } else if constexpr (std::is_same_v<CommandType, MetadataCommand>) {
            std::visit([&](auto command) { game.apply(std::move(command)); }, std::move(command));
          } else if constexpr (std::is_same_v<CommandType, InnerCommand>) {
            game.commands.push_back(std::move(command));
          } else {
            static_assert(std::is_same_v<CommandType, GameCommand>,
                          "Some command types are not considered");
          }
          return false;
        },
        commandSplit(std::move(readResult).unwrap()));
    if (breakFromLoop) {
      break;
    }
  }

  return Ok(std::move(game));
}

const std::vector<Board> &GameReader::capturedBoards() const {
  SOF_ASSERT_MSG("CaptureBoards option must be set", canCaptureBoards());
  return capturedBoards_;
}

Err<GameReader::Error> GameReader::error(std::string message) const {
  return Err(Error::error(line_, std::move(message)));
}

bool GameReader::canCaptureBoards() const {
  return (options_ & Options::CaptureBoards) != Options::None;
}

std::optional<std::string> GameReader::readLine() {
  std::string result;
  if (std::getline(*in_, result)) {
    ++line_;
    return result;
  }
  return std::nullopt;
}

auto GameReader::readCommand() -> CommandResult {
  for (;;) {
    if (auto result = tryReadCommand()) {
      return std::move(*result);
    }
  }
}

auto GameReader::tryReadCommand() -> std::optional<CommandResult> {
  using Ok = Ok<AnyCommand>;

#define D_VERIFY(cond, msg) \
  if (!(cond)) {            \
    return error(msg);      \
  }

  auto maybeLine = readLine();
  if (!maybeLine) {
    return Err(Error::endOfStream());
  }
  const std::string line(SoFUtil::trim(*maybeLine));
  if (line.empty() || line[0] == '#') {
    return std::nullopt;
  }

  const char *nameBegin = line.c_str();
  const char *nameEnd = SoFUtil::scanTokenEnd(nameBegin);
  const std::string_view name(nameBegin, nameEnd - nameBegin);

  const char *bodyBegin = SoFUtil::scanTokenStart(nameEnd);
  const char *bodyEnd = nameBegin + line.size();
  const std::string_view body(bodyBegin, bodyEnd - bodyBegin);

  // The code below assumes that the body is a null-terminated string
  SOF_ASSERT(*bodyEnd == '\0');

  if (name == "game") {
    const char *winnerBegin = bodyBegin;
    const char *winnerEnd = SoFUtil::scanTokenEnd(winnerBegin);
    const char *labelBegin = SoFUtil::scanTokenStart(winnerEnd);
    const char *labelEnd = SoFUtil::scanTokenEnd(labelBegin);
    D_VERIFY(winnerBegin + 1 == winnerEnd, "Winner must be a single character");
    D_VERIFY(labelBegin != labelEnd, "Label must be non-empty");
    auto winner = winnerFromChar(*winnerBegin);
    D_VERIFY(winner.has_value(), "Invalid winner character");
    auto label = std::make_optional<std::string>(labelBegin, labelEnd);
    if (*label == "-") {
      label = std::nullopt;
    }
    lastBoard_ = std::nullopt;
    return Ok(GameCommand{*winner, std::move(label)});
  }

  if (name == "title") {
    return Ok(TitleCommand{std::string(body)});
  }

  if (name == "start") {
    lastBoard_ = Board::initialPosition();
    if (canCaptureBoards()) {
      capturedBoards_.push_back(*lastBoard_);
    }
    return Ok(BoardCommand{makeCopyPtr<Board>(*lastBoard_)});
  }

  if (name == "board") {
    auto result = Board::fromFen(bodyBegin);
    if (result.isErr()) {
      return error(std::string("Error parsing FEN: ") + fenParseResultToStr(result.err()));
    }
    lastBoard_ = result.ok();
    if (canCaptureBoards()) {
      capturedBoards_.push_back(*lastBoard_);
    }
    return Ok(BoardCommand{makeCopyPtr<Board>(*lastBoard_)});
  }

  if (name == "moves") {
    D_VERIFY(lastBoard_.has_value(), "No preceding boards found");
    D_VERIFY(!body.empty(), "No moves specified");
    std::vector<Move> moves;
    for (const auto &srcMove : SoFUtil::split(bodyBegin)) {
      const Move move =
          SoFCore::moveParse(srcMove.data(), srcMove.data() + srcMove.size(), *lastBoard_);
      if (!move.isWellFormed(lastBoard_->side) || !isMoveValid(*lastBoard_, move) ||
          !isMoveLegal(*lastBoard_, move)) {
        return error("Move #" + std::to_string(moves.size() + 1) + " is illegal");
      }
      moves.push_back(move);
      moveMake(*lastBoard_, move);
      if (canCaptureBoards()) {
        capturedBoards_.push_back(*lastBoard_);
      }
    }
    return Ok(MovesCommand{std::move(moves)});
  }

  // Unknown command, just ignore it
  return std::nullopt;

#undef D_VERIFY
}

}  // namespace SoFGameSet
