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

#ifndef SOF_GAMESET_TYPES_INCLUDED
#define SOF_GAMESET_TYPES_INCLUDED

#include <iosfwd>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "core/board.h"
#include "core/move.h"
#include "util/copy_ptr.h"

namespace SoFGameSet {

// Winner of the chess game
enum class Winner { Unknown, White, Black, Draw };

// Parse winner from character `ch` in SoFGameSet format. If `ch` doesn't encode a winner in this
// format, return `nullopt`
std::optional<Winner> winnerFromChar(char ch);

// Convert winner to `char` in SoFGameSet format
char winnerToChar(Winner winner);

// "game" command
struct GameCommand {
  Winner winner;
  std::optional<std::string> label;

  // Write command to stream `out` in SoFGameSet format
  void write(std::ostream &out) const;
};

// "title" command
struct TitleCommand {
  std::string title;

  // Write command to stream `out` in SoFGameSet format
  void write(std::ostream &out) const;
};

// "board" (or "start") command
struct BoardCommand {
  // Must be non-null
  SoFUtil::CopyPtr<SoFCore::Board> board;

  // Write command to stream `out` in SoFGameSet format
  void write(std::ostream &out) const;
};

// "moves" command
struct MovesCommand {
  // Must be non-empty
  std::vector<SoFCore::Move> moves;

  // Write command to stream `out` in SoFGameSet format
  void write(std::ostream &out) const;
};

// Variant containing all possible commands
using AnyCommand = std::variant<GameCommand, TitleCommand, BoardCommand, MovesCommand>;

// Variant containing commands that can appear inside a game and do not encode metadata
using InnerCommand = std::variant<BoardCommand, MovesCommand>;

// Variant containing commands that appear inside a game and encode metadata
using MetadataCommand = std::variant<TitleCommand>;

// Determine the type of the command `command` and put the results into a variant according to the
// type
std::variant<GameCommand, MetadataCommand, InnerCommand> commandSplit(AnyCommand command);

// Write command `command` to stream `out` in SoFGameSet format
void commandWrite(std::ostream &out, const AnyCommand &command);
void commandWrite(std::ostream &out, const InnerCommand &command);

// Represents entire chess game in SoFGameSet format. A game is just a sequence of boards and is not
// required to be a valid chess game. You need to check separately whether the game is valid (e.g.
// by requiring that it's canonical in terms of SoFGameSet)
struct Game {
  // Game header, represented with a `GameCommand`
  GameCommand header;

  // Game title, if specified
  std::optional<std::string> title;

  // All the commands in the game, except the ones that specify metadata
  std::vector<InnerCommand> commands;

  Game() = default;
  explicit Game(GameCommand header) : header(std::move(header)) {}

  // Return `true` if the game is canonical in terms of SoFGameSet
  bool isCanonical() const;

  // Update metadata from the command `command`
  void apply(TitleCommand command);

  // Write the game to the stream `out` in SoFGameSet format
  void write(std::ostream &out) const;
};

}  // namespace SoFGameSet

#endif  // SOF_GAMESET_TYPES_INCLUDED
