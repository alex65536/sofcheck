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

#include "gameset/types.h"

#include <cstring>
#include <ostream>

#include "core/strutil.h"

namespace SoFGameSet {

std::optional<Winner> winnerFromChar(const char ch) {
  switch (ch) {
    case '?':
      return Winner::Unknown;
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

char winnerToChar(const Winner winner) {
  switch (winner) {
    case Winner::Unknown:
      return '?';
    case Winner::White:
      return 'W';
    case Winner::Black:
      return 'B';
    case Winner::Draw:
      return 'D';
  }
  SOF_UNREACHABLE();
}

void GameCommand::write(std::ostream &out) const {
  out << "game " << winnerToChar(winner) << " " << label.value_or("-") << "\n";
}

void TitleCommand::write(std::ostream &out) const { out << "title " << title << "\n"; }

void BoardCommand::write(std::ostream &out) const {
  static constexpr const char *INITIAL_POSITION_FEN =
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  char fen[SoFCore::BUFSZ_BOARD_FEN];
  board->asFen(fen);
  if (std::strcmp(fen, INITIAL_POSITION_FEN) == 0) {
    out << "start\n";
  } else {
    out << "board " << fen << "\n";
  }
}

void MovesCommand::write(std::ostream &out) const {
  out << "moves";
  for (const auto move : moves) {
    char moveStr[SoFCore::BUFSZ_MOVE_STR];
    moveToStr(move, moveStr);
    out << " " << moveStr;
  }
  out << "\n";
}

std::variant<GameCommand, MetadataCommand, InnerCommand> commandSplit(AnyCommand command) {
  return std::visit(
      [&](auto &&command) -> std::variant<GameCommand, MetadataCommand, InnerCommand> {
        return std::move(command);  // NOLINT(bugprone-move-forwarding-reference)
      },
      std::move(command));
}

void commandWrite(const AnyCommand &command, std::ostream &out) {
  std::visit([&](const auto &command) { command.write(out); }, command);
}

void commandWrite(const InnerCommand &command, std::ostream &out) {
  std::visit([&](const auto &command) { command.write(out); }, command);
}

bool Game::isCanonical() const {
  if (commands.empty()) {
    return false;
  }
  size_t boardCount = 0;
  for (const auto &command : commands) {
    if (std::holds_alternative<BoardCommand>(command)) {
      ++boardCount;
    }
  }
  return boardCount == 1;
}

void Game::apply(TitleCommand command) { title = std::move(command.title); }

void Game::write(std::ostream &out) const {
  header.write(out);
  if (title) {
    TitleCommand cmd{*title};
    cmd.write(out);
  }
  for (const auto &command : commands) {
    commandWrite(command, out);
  }
  out << "\n";
}

}  // namespace SoFGameSet
