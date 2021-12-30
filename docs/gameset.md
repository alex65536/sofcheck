# SoFCheck GameSet format (a.k.a. SoFGameSet) description

`SoFGameSet` is another format to store chess games. It is used to tune the evaluation weights in
SoFCheck: `make_dataset` takes a file in `SoFGameSet` format and generates a dataset available for
tuning

## Why a new format?

There exists many formats to store a bunch of games, most notable of which is PGN. But PGN is a
quite complex format, and moves in Standard Algebraic Notation are quite hard to parse. By
contrast, `SoFGameSet` format aims at:

- simplicity of format description
- simplicity of implementation in chess engines
- fast parsing
- reasonably small files

## Format description

This section describes version `1.0` of the format.

The file must be a valid UTF-8. Though, only ASCII is allowed in most places, unless specified
otherwise. Characters with ASCII code `0` **must not** appear in the file at all.

A _space character_ is the character with ASCII code `32`. Characters with ASCII codes from `1`
to `31` should not appear in the line, as they may or may not be considered space characters,
depending on the implementation. (For instance, SoFCheck also considers `\t` and `\r` spaces, but
you must not rely on it. Some time later, your files will just break if you don't follow the
rules.)

`SoFGameSet` is a line based format. Lines are separated using either `\n` or `\r\n` characters,
though `\n` line endings are preferred. Before processing, each line is _normalized_ by removing
any leading and trailing space characters. Then, all the empty lines and lines starting with `#`
are ignored. Processing of other lines is described below.

Each command consists of two parts, separated by one or more spaces: _command name_ and _command
body_. If _command name_ is not known to the parser, it must be ignored. _Command body_ can be
omitted, in this case the spaces after _command name_ are not necessary, and _command body_ is
considered empty.

Known command names (with the corresponding command bodies) are described below. A space in the
below description means one or more spaces.

- `game <winner> <label>`. Starts a new game with winner `<winner>` and label `<label>`. Everything
  after `<label>` must be ignored, as it is reserved for future use. Each game has a list of
  boards, initially empty. In the end, each game must have a non-empty list of boards. `<winner>`
  must be equal to either `W` (white wins), `B` (black wins), `D` (draw) or `?` (game not
  finished). `<label>` must consist of the characters from the set `[0-9a-zA-Z_]` and should
  uniquely identify each game in the file. If `<label>` is not specified, it must be equal to `-`.
- `title <title>`. Sets the current game title to `<title>`. `<title>` is arbitrary UTF-8 string
  that doesn't contain line endings.
- `start`. Adds the initial position to the list of boards.
- `board <board>`. Adds the position specified by FEN string `<board>` to the list of boards.
  `<board>` must be a valid FEN.
- `moves <moves>`. Adds new boards to the list, by applying the moves to the last board one by
  one. Thus, the number of added boards is equal to the number of provided moves. `<moves>` must be
  a list of valid moves in UCI format, separated by one or more spaces. There must be at least one
  board in the list of boards before this command.

A game is considered _canonical_ if there is at most one `title` command and exactly one command
from the set (`startpos`, `fen`). If the whole file contains only canonical games, it is considered
_canonical_.

## Example

The following file is canonical.

```
# Fool's mate: 1. g4 e5 f3 Qh4#.
game W fools_mate
start
moves g2g4 e7e5 f2f3 d8h4

# Okay, let's see a drawn game
# You will get a draw by repetitions
game D -
board 7k/4Q1p1/8/8/8/8/rrp5/2K5 w - - 0 1
moves e7e8 h8h7 e8h5 h7g8 h5e8 g8h7 e8h5 h7g8 h5e8 g8h7
```

## Implementation

Files in `SoFGameSet` format are produced by [Battlefield][1]. Versions `0.9.12` or later produce
canonical files, while earlier versions produce non-canonical files.

[1]: https://github.com/alex65536/sofcheck-engine-tester/tree/master/battlefield
