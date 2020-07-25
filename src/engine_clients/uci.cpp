#include "engine_clients/uci.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "core/move_parser.h"
#include "core/movegen.h"
#include "core/strutil.h"
#include "engine_base/options.h"
#include "engine_base/strutil.h"
#include "engine_base/types.h"
#include "engine_clients/private/uci_option_escape.h"
#include "util/misc.h"
#include "util/strutil.h"

namespace SoFEngineClients {

using SoFCore::Board;
using SoFCore::Move;
using SoFEngineBase::Options;
using SoFEngineBase::OptionType;
using SoFEngineBase::PositionCostBound;
using SoFEngineBase::PositionCostType;
using SoFUtil::panic;
using std::endl;
using std::pair;
using std::string;
using std::vector;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;

// Helper macro to return error in case of I/O errors
#define D_CHECK_IO(ioResult)     \
  {                              \
    if (!(ioResult)) {           \
      return ApiResult::IOError; \
    }                            \
  }

// Helper macro to return error in case of I/O errors (version for `poll()`)
#define D_CHECK_POLL_IO(ioResult) \
  {                               \
    if (!(ioResult)) {            \
      return PollResult::IOError; \
    }                             \
  }

void UciServerConnector::ensureClient() {
  if (unlikely(!client_)) {
    panic("The client is not connected");
  }
}

ApiResult UciServerConnector::finishSearch(const SoFCore::Move bestMove) {
  ensureClient();
  if (!searchStarted_) {
    return ApiResult::UnexpectedCall;
  }
  D_CHECK_IO(out_ << "bestmove " << SoFCore::moveToStr(bestMove) << endl);
  searchStarted_ = false;
  return ApiResult::Ok;
}

ApiResult UciServerConnector::reportError(const char *message) {
  ensureClient();
  err_ << "UCI client error: " << message << endl;
  D_CHECK_IO(out_ << "info string UCI client error: " << SoFUtil::sanitizeEol(message) << endl);
  return ApiResult::Ok;
}

ApiResult UciServerConnector::sendCurrMove(const SoFCore::Move move, const size_t moveNumber) {
  ensureClient();
  if (!searchStarted_) {
    return ApiResult::UnexpectedCall;
  }
  D_CHECK_IO(out_ << "info currmove " << SoFCore::moveToStr(move));
  if (moveNumber != 0) {
    D_CHECK_IO(out_ << " currmovenumber " << moveNumber);
  }
  D_CHECK_IO(out_ << endl);
  return ApiResult::Ok;
}

ApiResult UciServerConnector::sendHashFull(const SoFEngineBase::permille_t hashFull) {
  ensureClient();
  if (!searchStarted_) {
    return ApiResult::UnexpectedCall;
  }
  if (hashFull > 1000) {
    return ApiResult::InvalidArgument;
  }
  D_CHECK_IO(out_ << "info hashfull " << hashFull << endl);
  return ApiResult::Ok;
}

template <typename Duration>
inline static bool calcNodesPerSecond(const uint64_t nodes, const Duration time, uint64_t &nps) {
  const double timeSec = duration_cast<duration<double>>(time).count();
  if (timeSec < 1e-9) {
    return false;
  }
  const double npsFloat = std::round(static_cast<double>(nodes) / timeSec);
  // Check if the result fits into uint64_t
  if (npsFloat < 0 || npsFloat > static_cast<double>(std::numeric_limits<uint64_t>::max())) {
    return false;
  }
  nps = static_cast<uint64_t>(npsFloat);
  return true;
}

ApiResult UciServerConnector::sendNodeCount(const uint64_t nodes) {
  ensureClient();
  if (!searchStarted_) {
    return ApiResult::UnexpectedCall;
  }
  const auto time = getSearchTime();
  const uint64_t timeMsec = duration_cast<milliseconds>(time).count();
  D_CHECK_IO(out_ << "info nodes " << nodes << " time " << timeMsec);
  uint64_t nps = 0;
  if (calcNodesPerSecond(nodes, time, nps)) {
    D_CHECK_IO(out_ << " nps " << nps);
  }
  D_CHECK_IO(out_ << endl);
  return ApiResult::Ok;
}

ApiResult UciServerConnector::sendResult(const SoFEngineBase::SearchResult &result) {
  ensureClient();
  if (!searchStarted_) {
    return ApiResult::UnexpectedCall;
  }
  const uint64_t timeMsec = duration_cast<milliseconds>(getSearchTime()).count();
  D_CHECK_IO(out_ << "info depth " << result.depth << " time " << timeMsec);
  if (result.pvLen != 0) {
    D_CHECK_IO(out_ << " pv");
    for (size_t i = 0; i < result.pvLen; ++i) {
      D_CHECK_IO(out_ << " " << SoFCore::moveToStr(result.pv[i]));
    }
  }
  switch (result.cost.type()) {
    case PositionCostType::Centipawns: {
      D_CHECK_IO(out_ << " score cp " << result.cost.centipawns());
      break;
    }
    case PositionCostType::Checkmate: {
      D_CHECK_IO(out_ << " score mate " << result.cost.checkMate());
      break;
    }
  }
  switch (result.bound) {
    case PositionCostBound::Exact: {
      // Do nothing
      break;
    }
    case PositionCostBound::Lowerbound: {
      D_CHECK_IO(out_ << " lowerbound");
      break;
    }
    case PositionCostBound::Upperbound: {
      D_CHECK_IO(out_ << " upperbound");
      break;
    }
  }
  D_CHECK_IO(out_ << endl);
  return ApiResult::Ok;
}

ApiResult UciServerConnector::sendString(const char *str) {
  D_CHECK_IO(out_ << "info string " << SoFUtil::sanitizeEol(str) << endl);
  return ApiResult::Ok;
}

ApiResult UciServerConnector::checkClient(ApiResult result) {
  if (result == ApiResult::Ok || result == ApiResult::NotSupported) {
    return result;
  }
  err_ << "UCI client error: " << SoFEngineBase::apiResultToStr(result) << endl;
  return result;
}

PollResult UciServerConnector::doStartSearch(const ApiResult searchStartResult) {
  if (searchStartResult != ApiResult::Ok) {
    const char *strResult = SoFEngineBase::apiResultToStr(searchStartResult);
    err_ << "UCI client error: Cannot start search: " << strResult << endl;
    D_CHECK_POLL_IO(out_ << "info string Cannot start search: " << strResult << endl);
    // We cannot start search from our side because of API call error. But it's better to tell the
    // server that we stopped (by sending null move). Otherwise the server will wait for the search
    // result infinitely.
    D_CHECK_POLL_IO(out_ << "bestmove 0000" << endl);
    return PollResult::Ok;
  }
  searchStarted_ = true;
  searchStartTime_ = std::chrono::steady_clock::now();
  return PollResult::Ok;
}

template <typename T>
bool UciServerConnector::tryReadInt(T &val, std::istream &stream, const char *intType) {
  string token;
  if (!(stream >> token)) {
    err_ << "UCI server error: Expected token, but end of line found" << endl;
    return false;
  }
  if (!SoFUtil::valueFromStr(token.data(), token.data() + token.size(), val)) {
    err_ << "UCI server error: Cannot interpret token \"" << token << "\" as " << intType << endl;
    return false;
  }
  return true;
}

bool UciServerConnector::tryReadMsec(milliseconds &time, std::istream &stream) {
  uint64_t val;
  if (!tryReadInt(val, stream, "uint64")) {
    return false;
  }
  if (val > static_cast<uint64_t>(milliseconds::max().count())) {
    err_ << "UCI server error: Value " << val << " is too large" << endl;
  }
  time = milliseconds(val);
  return true;
}

PollResult UciServerConnector::processUciGo(std::istream &tokens) {
  if (searchStarted_) {
    err_ << "UCI server error: Search is already started" << endl;
    return PollResult::NoData;
  }

  // List of supported subcommands
  const vector<string> subcommands{"searchmoves", "ponder", "wtime",     "btime",
                                   "winc",        "binc",   "movestogo", "depth",
                                   "nodes",       "mate",   "movetime",  "infinite"};

  // We don't support intricate combination of the parameters in this command, so we try to find
  // "depth", "nodes", "movetime" or "infinite" and call appropriate APIs to handle this. If nothing
  // from this list is encountered, we just assume that the time parameters are given and try to use
  // `searchTimeControl`. Some subcommands are not supported now, and they are just ignored.
  //
  // If such parser behaviour causes bugs in some GUIs, feel free to report a bug and send the
  // string received by the engine, and I will patch this logic to include such weird cases.
  bool hasTimeControl = false;
  SoFEngineBase::TimeControl timeControl;
  for (;;) {
    string token;
    if (!(tokens >> token)) {
      break;
    }
    if (token == "searchmoves") {
      // Not supported, skip any token until we get either a valid subcommand or end of line
      for (;;) {
        if (!(tokens >> token)) {
          // Found end of line
          token = "";
          break;
        }
        if (std::find(subcommands.begin(), subcommands.end(), token) != subcommands.end()) {
          // Found valid subcommand
          break;
        }
      }
      // We read an extra token here. We need to fall through and parse it
    }
    if (token == "searchmoves") {
      // Two "searchmoves" in a row is really weird
      err_ << "UCI server error: \"searchmoves\" is mentioned twice in a row" << endl;
      continue;
    }
    if (token == "ponder") {
      // Not supported.
      continue;
    }
    if (token == "wtime") {
      hasTimeControl |= tryReadMsec(timeControl.white.time, tokens);
      continue;
    }
    if (token == "btime") {
      hasTimeControl |= tryReadMsec(timeControl.black.time, tokens);
      continue;
    }
    if (token == "winc") {
      hasTimeControl |= tryReadMsec(timeControl.white.inc, tokens);
      continue;
    }
    if (token == "binc") {
      hasTimeControl |= tryReadMsec(timeControl.black.inc, tokens);
      continue;
    }
    if (token == "movestogo") {
      uint64_t val;
      if (!tryReadInt(val, tokens, "uint64")) {
        continue;
      }
      timeControl.movesToGo = val;
      hasTimeControl = true;
      continue;
    }
    if (token == "depth") {
      // Fixed depth.
      size_t val;
      if (!tryReadInt(val, tokens, "size_t")) {
        continue;
      }
      return doStartSearch(client_->searchFixedDepth(val));
    }
    if (token == "nodes") {
      // Fixed nodes.
      uint64_t val;
      if (!tryReadInt(val, tokens, "uint64")) {
        continue;
      }
      return doStartSearch(client_->searchFixedNodes(val));
    }
    if (token == "mate") {
      // Not supported.
      string unused;
      tokens >> unused;
      continue;
    }
    if (token == "movetime") {
      milliseconds val;
      if (!tryReadMsec(val, tokens)) {
        continue;
      }
      return doStartSearch(client_->searchFixedTime(val));
    }
    if (token == "infinite") {
      return doStartSearch(client_->searchInfinite());
    }
    if (token.empty()) {
      // Empty token means end of line; exit the parser loop
      break;
    }
  }

  if (!hasTimeControl) {
    // If no suitable search type found, then go infinite
    err_ << "UCI server warning: No useful parameters specified for \"go\"; running infinite search"
         << endl;
    return doStartSearch(client_->searchInfinite());
  }

  // Run with time control
  return doStartSearch(client_->searchTimeControl(timeControl));
}

PollResult UciServerConnector::processUciPosition(std::istream &tokens) {
  // Scan the position description (until the end of line or "moves" token)
  string fenString;
  string token;
  while (tokens >> token) {
    if (token == "moves") {
      break;
    }
    if (!fenString.empty()) {
      fenString += ' ';
    }
    fenString += token;
  }

  // Convert the position description into `Board` structure
  Board board;  // NOLINT : the board will be initialized below
  if (fenString == "startpos") {
    board.setInitialPosition();
  } else {
    SoFCore::FenParseResult parseRes = board.setFromFen(fenString.c_str());
    if (parseRes != SoFCore::FenParseResult::Ok) {
      err_ << "UCI server error: Cannot parse position \"" << fenString
           << "\" << : " << SoFCore::fenParseResultToStr(parseRes) << endl;
      return PollResult::NoData;
    }
    SoFCore::ValidateResult validateRes = board.validate();
    if (validateRes != SoFCore::ValidateResult::Ok) {
      err_ << "UCI server error: Position \"" << fenString
           << "\" is invalid: " << SoFCore::validateResultToStr(validateRes) << endl;
      return PollResult::NoData;
    }
  }

  // Parse all the moves and apply them to `dstBoard`
  Board dstBoard = board;
  vector<Move> moves;
  while (tokens >> token) {
    const Move move = SoFCore::moveParse(token.c_str(), dstBoard);
    if (!move.isWellFormed(dstBoard.side) || !SoFCore::isMoveValid(dstBoard, move)) {
      err_ << "UCI server error: Move \"" << token << "\" is invalid" << endl;
      return PollResult::NoData;
    }
    moves.push_back(move);
    SoFCore::moveMake(dstBoard, move);
    if (!isMoveLegal(dstBoard)) {
      err_ << "UCI server error: Move \"" << token << "\" is not legal" << endl;
      return PollResult::NoData;
    }
  }

  // Finally, after everything is parsed, just call client API
  checkClient(client_->setPosition(board, moves.data(), moves.size()));
  return PollResult::Ok;
}

PollResult UciServerConnector::listOptions() {
  const Options &opts = client_->options();
  vector<pair<string, OptionType>> keys = opts.list();
  std::sort(keys.begin(), keys.end());
  for (const auto &[key, type] : keys) {
    D_CHECK_POLL_IO(out_ << "option name " << Private::uciOptionNameEscape(key) << " type");
    switch (type) {
      case OptionType::Bool: {
        const auto *option = opts.getBool(key);
        D_CHECK_POLL_IO(out_ << " check default " << (option->value ? "true" : "false") << endl);
        break;
      }
      case OptionType::Enum: {
        const auto *option = opts.getEnum(key);
        D_CHECK_POLL_IO(out_ << " combo default "
                             << Private::uciEnumNameEscape(option->items[option->index]));
        for (const string &item : option->items) {
          D_CHECK_POLL_IO(out_ << " val " << Private::uciEnumNameEscape(item));
        }
        D_CHECK_POLL_IO(out_ << endl);
        break;
      }
      case OptionType::Int: {
        const auto *option = opts.getInt(key);
        D_CHECK_POLL_IO(out_ << " spin default " << option->value << " min " << option->minValue
                             << " max " << option->maxValue << endl);
        break;
      }
      case OptionType::String: {
        const auto *option = opts.getString(key);
        string value = SoFUtil::sanitizeEol(option->value);
        // If the string is empty or consists only of spaces, then print special value <empty>
        if (*SoFUtil::scanTokenStart(value.c_str()) == '\0') {
          value = "<empty>";
        }
        D_CHECK_POLL_IO(out_ << " string default " << value << endl);
        break;
      }
      case OptionType::Action: {
        D_CHECK_POLL_IO(out_ << " button" << endl);
        break;
      }
      case OptionType::None: {
        unreachable();
        break;
      }
    }
  }
  return PollResult::Ok;
}

PollResult UciServerConnector::processUciSetOption(std::istream &tokens) {
  // Read "name" token. There must be such token, according to UCI docs. If not, just assume that
  // the first token is the start of the option name
  string name;
  string token;
  if (!(tokens >> token)) {
    err_ << "UCI server error: cannot read option name" << endl;
    return PollResult::NoData;
  }
  if (token != "name") {
    err_ << R"R(UCI server error: "name" token expected)R" << endl;
    name += token;
  }

  // Seek for end of line or "value" token (which denotes end of the option name)
  for (;;) {
    if (!(tokens >> token) || token == "value") {
      break;
    }
    if (!name.empty()) {
      name += ' ';
    }
    name += token;
  }
  name = Private::uciOptionNameUnescape(name);

  OptionType type = client_->options().type(name);
  if (type == OptionType::None) {
    err_ << "UCI server error: no such option \"" << name << "\"" << endl;
    return PollResult::NoData;
  }

  // Read value
  string value;
  if (type == OptionType::String) {
    // String must be read without parsing it into tokens
    std::getline(tokens, value);
    value = SoFUtil::trim(value);
  } else {
    // Parse value as tokens
    string token;
    while (tokens >> token) {
      if (!value.empty()) {
        value += ' ';
      }
      value += token;
    }
  }

  switch (type) {
    case OptionType::Bool: {
      if (value == "0" || value == "false") {
        checkClient(client_->options().setBool(name, false));
        return PollResult::Ok;
      }
      if (value == "1" || value == "true") {
        checkClient(client_->options().setBool(name, true));
        return PollResult::Ok;
      }
      err_ << R"R(UCI server error: expected "0", "1", "true" or "false", ")R" << value
           << "\" found" << endl;
      return PollResult::NoData;
    }
    case OptionType::Int: {
      int64_t result;
      if (!SoFUtil::valueFromStr(value.c_str(), value.c_str() + value.size(), result)) {
        err_ << "UCI server error: \"" << value << "\" is not int64" << endl;
        return PollResult::NoData;
      }
      checkClient(client_->options().setInt(name, result));
      return PollResult::Ok;
    }
    case OptionType::String: {
      checkClient(client_->options().setString(name, (value == "<empty>") ? std::string() : value));
      return PollResult::Ok;
    }
    case OptionType::Enum: {
      checkClient(client_->options().setEnum(name, Private::uciEnumNameUnescape(value)));
      return PollResult::Ok;
    }
    case OptionType::Action: {
      checkClient(client_->options().triggerAction(name));
      return PollResult::Ok;
    }
    case OptionType::None: {
      unreachable();
    }
  }

  panic("Unknown option type, end of function reached in processUciSetOption");
}

PollResult UciServerConnector::poll() {
  // Check that the client is connected
  ensureClient();

  // Read command line
  string cmdLine;
  std::getline(in_, cmdLine);
  D_CHECK_POLL_IO(!in_.bad());
  if (in_.eof()) {
    err_ << "Stopping." << endl;
    return PollResult::Shutdown;
  }
  if (cmdLine.empty()) {
    return PollResult::NoData;
  }

  // Scan for command name. UCI standard says that we can safely skip all the words we don't know
  // and use the first known word as UCI command. For example, "joho debug on\n" will be
  // interperted just like "debug on\n"
  std::istringstream cmdTokens(cmdLine);
  string command;
  while (cmdTokens >> command) {
    if (command == "uci") {
      D_CHECK_POLL_IO(out_ << "id name " << SoFUtil::sanitizeEol(client_->name()) << endl);
      D_CHECK_POLL_IO(out_ << "id author " << SoFUtil::sanitizeEol(client_->author()) << endl);
      PollResult res = listOptions();
      if (res != PollResult::Ok) {
        return res;
      }
      D_CHECK_POLL_IO(out_ << "uciok" << endl);
      return PollResult::Ok;
    }
    if (command == "debug") {
      // Expecting "on" or "off"
      string value;
      cmdTokens >> value;
      if (value != "on" && value != "off") {
        err_ << R"(UCI server error: Tokens "on" or "off" expected after "debug")" << endl;
        return PollResult::NoData;
      }
      const bool newDebugEnabled = (value == "on");
      if (debugEnabled_ == newDebugEnabled) {
        err_ << "UCI server error: Debug is already " << (debugEnabled_ ? "enabled" : "disabled")
             << endl;
        return PollResult::NoData;
      }
      if (newDebugEnabled) {
        client_->enterDebugMode();
      } else {
        client_->leaveDebugMode();
      }
      debugEnabled_ = newDebugEnabled;
      return PollResult::Ok;
    }
    if (command == "isready") {
      // Always reply "readyok"
      D_CHECK_POLL_IO(out_ << "readyok" << endl);
      return PollResult::Ok;
    }
    if (command == "setoption") {
      return processUciSetOption(cmdTokens);
    }
    if (command == "register") {
      // Not supported.
      return PollResult::NoData;
    }
    if (command == "ucinewgame") {
      checkClient(client_->newGame());
      return PollResult::Ok;
    }
    if (command == "position") {
      return processUciPosition(cmdTokens);
    }
    if (command == "go") {
      return processUciGo(cmdTokens);
    }
    if (command == "stop") {
      if (!searchStarted_) {
        err_ << "UCI server error: Cannot stop search, as it is not started" << endl;
        return PollResult::NoData;
      }
      checkClient(client_->stopSearch());
      return PollResult::Ok;
    }
    if (command == "ponderhit") {
      // Not supported.
      return PollResult::NoData;
    }
    if (command == "quit") {
      err_ << "Stopping." << endl;
      return PollResult::Shutdown;
    }
    // Unknown word, just skip it
  }

  // The given string didn't contain any commands
  err_ << "UCI server error: Cannot interpret line as UCI command" << endl;
  return PollResult::NoData;
}

ApiResult UciServerConnector::connect(SoFEngineBase::Client *client) {
  if (unlikely(client_)) {
    panic("The client is already connected");
  }
  client_ = client;
  return ApiResult::Ok;
}

void UciServerConnector::disconnect() {
  ensureClient();
  client_ = nullptr;
}

UciServerConnector::UciServerConnector() : UciServerConnector(std::cin, std::cout, std::cerr) {}

UciServerConnector::UciServerConnector(std::istream &in, std::ostream &out, std::ostream &err)
    : searchStarted_(false),
      debugEnabled_(false),
      client_(nullptr),
      in_(in),
      out_(out),
      err_(err) {}

UciServerConnector::~UciServerConnector() {
  if (unlikely(client_)) {
    panic("Client was not disconnected properly");
  }
}

}  // namespace SoFEngineClients
