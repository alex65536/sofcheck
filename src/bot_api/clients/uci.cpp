#include "bot_api/clients/uci.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "bot_api/clients/private/uci_option_escape.h"
#include "bot_api/options.h"
#include "bot_api/strutil.h"
#include "bot_api/types.h"
#include "core/move_parser.h"
#include "core/movegen.h"
#include "core/strutil.h"
#include "util/logging.h"
#include "util/misc.h"
#include "util/strutil.h"

namespace SoFBotApi::Clients {

using SoFCore::Board;
using SoFCore::Move;
using SoFUtil::logError;
using SoFUtil::logInfo;
using SoFUtil::logWarn;
using SoFUtil::panic;
using std::endl;
using std::pair;
using std::string;
using std::vector;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;

// Types of log entries
constexpr const char *UCI_CLIENT = "UCI client";
constexpr const char *UCI_SERVER = "UCI server";

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
  if (SOF_UNLIKELY(!client_)) {
    panic("The client is not connected");
  }
}

ApiResult UciServerConnector::finishSearch(const SoFCore::Move bestMove) {
  ensureClient();
  std::lock_guard guard(mutex_);
  if (!searchStarted_) {
    return ApiResult::UnexpectedCall;
  }
  D_CHECK_IO(out_ << "bestmove " << SoFCore::moveToStr(bestMove) << endl);
  searchStarted_ = false;
  return ApiResult::Ok;
}

ApiResult UciServerConnector::reportError(const char *message) {
  ensureClient();
  std::lock_guard guard(mutex_);
  logError(UCI_CLIENT) << message;
  D_CHECK_IO(out_ << "info string UCI client error: " << SoFUtil::sanitizeEol(message) << endl);
  return ApiResult::Ok;
}

ApiResult UciServerConnector::sendCurrMove(const SoFCore::Move move, const size_t moveNumber) {
  ensureClient();
  std::lock_guard guard(mutex_);
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

ApiResult UciServerConnector::sendHashFull(const permille_t hashFull) {
  ensureClient();
  std::lock_guard guard(mutex_);
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
  if (npsFloat < 0 || npsFloat >= static_cast<double>(std::numeric_limits<uint64_t>::max())) {
    return false;
  }
  nps = static_cast<uint64_t>(npsFloat);
  return true;
}

ApiResult UciServerConnector::sendNodeCount(const uint64_t nodes) {
  ensureClient();
  std::lock_guard guard(mutex_);
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

ApiResult UciServerConnector::sendResult(const SearchResult &result) {
  ensureClient();
  std::lock_guard guard(mutex_);
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
  ensureClient();
  std::lock_guard guard(mutex_);
  D_CHECK_IO(out_ << "info string " << SoFUtil::sanitizeEol(str) << endl);
  return ApiResult::Ok;
}

ApiResult UciServerConnector::checkClient(ApiResult result) {
  if (result == ApiResult::Ok || result == ApiResult::NotSupported) {
    return result;
  }
  logError(UCI_CLIENT) << apiResultToStr(result);
  return result;
}

PollResult UciServerConnector::doStartSearch(const ApiResult searchStartResult) {
  if (searchStartResult != ApiResult::Ok) {
    const char *strResult = apiResultToStr(searchStartResult);
    logError(UCI_CLIENT) << "Cannot start search: " << strResult;
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
    logError(UCI_SERVER) << "Expected token, but end of line found";
    return false;
  }
  if (!SoFUtil::valueFromStr(token.data(), token.data() + token.size(), val)) {
    logError(UCI_SERVER) << "Cannot interpret token \"" << token << "\" as " << intType;
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
    logError(UCI_SERVER) << "Value " << val << " is too large";
    return false;
  }
  time = milliseconds(val);
  return true;
}

PollResult UciServerConnector::processUciGo(std::istream &tokens) {
  if (searchStarted_) {
    logError(UCI_SERVER) << "Search is already started";
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
  TimeControl timeControl;
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
      logWarn(UCI_SERVER) << "\"searchmoves\" is mentioned twice in a row";
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
      hasTimeControl |= tryReadInt(timeControl.movesToGo, tokens, "uint64");
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
    logWarn(UCI_SERVER) << "No useful parameters specified for \"go\"; running infinite search";
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
      logError(UCI_SERVER) << "Cannot parse position \"" << fenString
                           << "\" << : " << SoFCore::fenParseResultToStr(parseRes);
      return PollResult::NoData;
    }
    SoFCore::ValidateResult validateRes = board.validate();
    if (validateRes != SoFCore::ValidateResult::Ok) {
      logError(UCI_SERVER) << "Position \"" << fenString
                           << "\" is invalid: " << SoFCore::validateResultToStr(validateRes);
      return PollResult::NoData;
    }
  }

  // Parse all the moves and apply them to `dstBoard`
  Board dstBoard = board;
  vector<Move> moves;
  while (tokens >> token) {
    const Move move = SoFCore::moveParse(token.c_str(), dstBoard);
    if (!move.isWellFormed(dstBoard.side) || !SoFCore::isMoveValid(dstBoard, move)) {
      logError(UCI_SERVER) << "Move \"" << token << "\" is invalid";
      return PollResult::NoData;
    }
    moves.push_back(move);
    SoFCore::moveMake(dstBoard, move);
    if (!SoFCore::isMoveLegal(dstBoard)) {
      logError(UCI_SERVER) << "Move \"" << token << "\" is not legal";
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
        SOF_UNREACHABLE();
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
    logError(UCI_SERVER) << "Cannot read option name";
    return PollResult::NoData;
  }
  if (token != "name") {
    logWarn(UCI_SERVER) << "\"name\" token expected";
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

  Options &opts = client_->options();
  OptionType type = opts.type(name);
  if (type == OptionType::None) {
    logError(UCI_SERVER) << "No such option \"" << name << "\"";
    return PollResult::NoData;
  }

  // Read value
  string value;
  if (type == OptionType::String) {
    // String must be read without parsing it into tokens
    std::getline(tokens, value);
    value = SoFUtil::trim(value);
  } else if (type == OptionType::Enum) {
    // Parse value as tokens
    string token;
    while (tokens >> token) {
      if (!value.empty()) {
        value += ' ';
      }
      value += token;
    }
  } else if (type != OptionType::Action) {
    // Extract single token
    if (!(tokens >> value)) {
      value = "";
    }
  }

  switch (type) {
    case OptionType::Bool: {
      if (value == "0" || value == "false") {
        checkClient(opts.setBool(name, false));
        return PollResult::Ok;
      }
      if (value == "1" || value == "true") {
        checkClient(opts.setBool(name, true));
        return PollResult::Ok;
      }
      logError(UCI_SERVER) << R"R(Expected "0", "1", "true" or "false", ")R" << value << "\" found";
      return PollResult::NoData;
    }
    case OptionType::Int: {
      int64_t result;
      if (!SoFUtil::valueFromStr(value.c_str(), value.c_str() + value.size(), result)) {
        logError(UCI_SERVER) << "\"" << value << "\" is not int64";
        return PollResult::NoData;
      }
      checkClient(opts.setInt(name, result));
      return PollResult::Ok;
    }
    case OptionType::String: {
      checkClient(opts.setString(name, (value == "<empty>") ? std::string() : value));
      return PollResult::Ok;
    }
    case OptionType::Enum: {
      checkClient(opts.setEnum(name, Private::uciEnumNameUnescape(value)));
      return PollResult::Ok;
    }
    case OptionType::Action: {
      checkClient(opts.triggerAction(name));
      return PollResult::Ok;
    }
    case OptionType::None: {
      SOF_UNREACHABLE();
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
    logInfo(UCI_SERVER) << "Stopping.";
    return PollResult::Shutdown;
  }
  if (cmdLine.empty()) {
    return PollResult::NoData;
  }

  std::istringstream tokens(cmdLine);
  return processUciCommand(tokens);
}

PollResult UciServerConnector::processUciCommand(std::istream &tokens) {
  std::lock_guard guard(mutex_);

  // Scan for command name. UCI standard says that we can safely skip all the words we don't know
  // and use the first known word as UCI command. For example, "joho debug on\n" will be
  // interperted just like "debug on\n"
  string command;
  while (tokens >> command) {
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
      // Expect one of the tokens "on" or "off"
      string value;
      tokens >> value;
      if (value != "on" && value != "off") {
        logError(UCI_SERVER) << R"(Token "on" or "off" expected after "debug")";
        return PollResult::NoData;
      }
      const bool newDebugEnabled = (value == "on");
      if (debugEnabled_ == newDebugEnabled) {
        logWarn(UCI_SERVER) << "Debug is already " << (debugEnabled_ ? "enabled" : "disabled");
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
      return processUciSetOption(tokens);
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
      return processUciPosition(tokens);
    }
    if (command == "go") {
      return processUciGo(tokens);
    }
    if (command == "stop") {
      if (!searchStarted_) {
        logError(UCI_SERVER) << "Cannot stop search, as it is not started";
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
      logInfo(UCI_SERVER) << "Stopping.";
      return PollResult::Shutdown;
    }
    // Unknown word, just skip it
  }

  // The given string didn't contain any commands
  logError(UCI_SERVER) << "Cannot interpret line as UCI command";
  return PollResult::NoData;
}

ApiResult UciServerConnector::connect(Client *client) {
  if (SOF_UNLIKELY(client_)) {
    panic("The client is already connected");
  }
  client_ = client;
  return ApiResult::Ok;
}

void UciServerConnector::disconnect() {
  ensureClient();
  client_ = nullptr;
}

UciServerConnector::UciServerConnector() : UciServerConnector(std::cin, std::cout) {}

UciServerConnector::UciServerConnector(std::istream &in, std::ostream &out)
    : searchStarted_(false), debugEnabled_(false), client_(nullptr), in_(in), out_(out) {}

UciServerConnector::~UciServerConnector() {
  if (SOF_UNLIKELY(client_)) {
    panic("Client was not disconnected properly");
  }
}

}  // namespace SoFBotApi::Clients
