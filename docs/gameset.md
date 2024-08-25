# SoFCheck GameSet (a.k.a. SoFGameSet) format description

`SoFGameSet` is another format to store chess games. It is used to tune the evaluation weights in
SoFCheck: `make_dataset` takes a file in `SoFGameSet` format and generates a dataset available for
tuning.

## Why a new format?

There exist many formats to store chess games, most notable of which is PGN. But PGN is a quite
complex format, and moves in Standard Algebraic Notation are quite hard to parse.

By contrast, `SoFGameSet` sets the following goals, in order from most to least important:

- simplicity of implementation in chess engines
- fast parsing
- simplicity of format description
- reasonably small files

## Format description

This section describes version `1.0` of the format.

### Definitions

The specification below uses the words _should_ and _must_. "Must" means that any valid file must
follow the rules, and parsers must enforce it (i.e. by throwing an error when a file breaks these
rules). "Should" also means that any valid file must follow the rules, but in case of error the
behaviour of a parser is implementation-defined. It may silently ignore the error and interpret
the file in an unspecified way. It may also throw the error.

### Games and boards

The file encodes a list of _games_. Each game is a list of _boards_ that occur sequentially in the
game. The first board is the starting position. Additional game properties can be also stored in
the file.

It's not required that a game in the file encodes a valid chess game. Though, if the file is
canonical (see below), all the games are valid chess games by definition. Validity is not required,
because the file may keep, for example, some subset of chess game positions, not the entire games.

### General structure

The file should be a valid UTF-8. Though, non-ASCII characters should appear only in the places
where it's explicitly allowed by this specification. Characters with ASCII code `0` should not
appear in the file at all.

_Space character_ is a character with ASCII code `32`. Characters with ASCII codes from `1` to
`31` should not appear in the line, as they may or may not be considered space characters,
depending on the implementation. (For instance, SoFCheck assumes that `\t` and `\r` are also
spaces, but don't rely on it.)

The file format is case-sensitive. So, all the commands (see below) must be written in lower case,
as in this specification.

`SoFGameSet` is a line based format. Lines are separated using either LF or CRLF line endings,
though LF line endings are preferred. Before processing, each line is _normalized_ by removing
any leading and trailing space characters. Then, all the empty lines and lines starting with `#`
are ignored. Other lines are parsed in order from first to last and must follow the follow the
format below.

### Line format

Each line contains a command. Each command must consist of two parts: _command name_ and _command
body_. If command name is not known to the parser, it must be ignored. Command body can be omitted,
in this case the spaces after command name are not necessary, and command body is considered empty.

Known command names, along with the format of corresponding command bodies, are described below. A
space in this description means one or more spaces in the file.

- `game <winner> <label>`: Opens a new game with empty list of boards, winner `<winner>` and label
  `<label>`. The text after `<label>` must be ignored by parser, as it is reserved for future use.
  `<winner>` must be equal to either `W` (white wins), `B` (black wins), `D` (draw) or `?` (game
  not finished). `<label>` should consist of no more than `64` characters, each of them should be
  from the set `[0-9a-zA-Z_]`. The label should uniquely identify each game in the file. If
  `<label>` is not specified, it must be equal to `-`. Note that there can be arbitrarily many
  games with an unspecified label.
- `title <title>`: Sets the current game title to `<title>`. `<title>` can be arbitrary UTF-8
  string (with regard to the restrictions mentioned above). Of course, `<title>` must not contain
  line endings. Title should be no more than `4096` UTF-8 characters long.
- `start`: Appends the initial position to the list of boards.
- `board <board>`: Appends the position specified by FEN string `<board>` to the list of boards.
  `<board>` must be a valid FEN.
- `moves <moves>`: Appends new boards to the list, by applying the specified moves to the last
  board one by one. Thus, the number of added boards is equal to the number of provided moves.
  `<moves>` must be a list of valid moves in UCI format, separated by one or more spaces. There
  must be at least one board in the list of boards before this command.

### Canonical and non-canonical games

A game is _canonical_ if there is exactly one command from the set (`start`, `board`). Note that
canonical games by definition correspond to valid chess games.

A file is _canonical_ if it contains only canonical games.

### Example

Example of canonical file:

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
