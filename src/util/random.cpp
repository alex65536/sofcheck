// This file is part of SoFCheck
//
// Copyright (c) 2020 Alexander Kernozhitsky and SoFCheck contributors
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

#include "util/random.h"

#include <chrono>
#include <random>
#include <thread>

#include "config.h"
#include "util/misc.h"

namespace SoFUtil {

static uint64_t makeRandomSeed() {
  return std::chrono::steady_clock::now().time_since_epoch().count() +
         static_cast<uint64_t>(std::hash<std::thread::id>()(std::this_thread::get_id()));
}

struct RandGenWrapper {
  std::mt19937_64 gen;

  RandGenWrapper() noexcept : gen(makeRandomSeed()) {}
};

uint64_t random() {
  static thread_local RandGenWrapper wrapper;
  return wrapper.gen();
}

}  // namespace SoFUtil
