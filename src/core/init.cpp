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

#include "core/init.h"

#include <atomic>

#include "core/private/magic.h"
#include "core/private/zobrist.h"

namespace SoFCore {

void init() {
  static std::atomic_flag initialized;
  if (initialized.test_and_set()) {
    return;
  }
  Private::initMagic();
  Private::initZobrist();
}

}  // namespace SoFCore
