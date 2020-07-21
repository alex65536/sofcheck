#include "engine_clients/uci.h"

#include <cmath>
#include <iostream>
#include <limits>

#include "core/strutil.h"
#include "util/misc.h"
#include "util/strutil.h"

namespace SoFEngineClients {

using SoFEngineBase::PositionCostBound;
using SoFEngineBase::PositionCostType;
using SoFUtil::panic;
using std::endl;
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
  D_CHECK_IO(err_ << "UCI client error: " << message << endl);
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

SoFEngineBase::PollResult UciServerConnector::poll() {
  // TODO : read commands from the server!
  return PollResult::RuntimeError;
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
    : searchStarted_(false), client_(nullptr), in_(in), out_(out), err_(err) {}

UciServerConnector::~UciServerConnector() {
  if (unlikely(client_)) {
    panic("Client was not disconnected properly");
  }
}

}  // namespace SoFEngineClients
