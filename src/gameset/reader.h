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

#ifndef SOF_GAMESET_READER_INCLUDED
#define SOF_GAMESET_READER_INCLUDED

#include <iosfwd>
#include <string>

#include "gameset/types.h"
#include "util/no_copy_move.h"
#include "util/operators.h"
#include "util/result.h"

namespace SoFGameSet {

// Reader for SoFGameSet file format
class GameReader : public SoFUtil::NoCopy {
public:
  // Reader options
  enum class Options : int32_t {
    None = 0,
    // If set, then the reader saves all the boards which appear in the game. These boards can be
    // obtained by calling `capturedBoards()`
    CaptureBoards = 1,
    All = 1,
  };

  // Reader error
  struct Error {
    // Error type
    enum class Status {
      // There are no more games in the stream
      EndOfStream,
      // Error while reading. `line` is the line number on which the error has happened, and
      // `message` is the error message
      Error,
    };

    Status status;
    size_t line;
    std::string message;

    // Create `Error` of type `Status::Error`
    static Error error(const size_t line, std::string message) {
      return Error{Status::Error, line, std::move(message)};
    }

    // Create `Error` of type `Status::EndOfStream`
    static Error endOfStream() { return Error{Status::EndOfStream, 0, ""}; }
  };

  explicit GameReader(std::istream &in, Options options = Options::None);

  // Read the next game from the stream
  SoFUtil::Result<Game, Error> nextGame();

  // Get number of lines that were already read
  size_t lineCount() const { return line_; }

  // Call this function only if the last call to `nextGame()` finished successfully, and
  // `CaptureBoards` is set in the options. It will return all the boards from the last game, in the
  // same order in which they appear
  const std::vector<SoFCore::Board> &capturedBoards() const;

private:
  using CommandResult = SoFUtil::Result<AnyCommand, Error>;

  // Return `true` if we need to save all the boards in the game
  bool canCaptureBoards() const;

  // Helper function to create `Error` of type `Status::Error` from the message `message`
  SoFUtil::Err<Error> error(std::string message) const;

  // Read the next line from the stream. If there are no more lines, return `nullopt`
  std::optional<std::string> readLine();

  // Read the next line and try to interpret it as a command. If this line must be skipped, return
  // `nullopt`. Otherwise return either the parsed command, or error
  std::optional<CommandResult> tryReadCommand();

  // Read next command from the stream
  CommandResult readCommand();

  std::istream *in_;
  Options options_;
  size_t line_ = 0;
  CommandResult lastCommand_ = SoFUtil::Err(Error::endOfStream());
  std::optional<SoFCore::Board> lastBoard_ = std::nullopt;
  std::vector<SoFCore::Board> capturedBoards_;
};

SOF_ENUM_BITWISE(GameReader::Options, int32_t);

}  // namespace SoFGameSet

#endif  // SOF_GAMESET_READER_INCLUDED
