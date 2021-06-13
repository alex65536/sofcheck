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

#ifndef SOF_UTIL_DEFER_INCLUDED
#define SOF_UTIL_DEFER_INCLUDED

#include <memory>

#include "util/misc.h"
#include "util/no_copy_move.h"

namespace SoFUtil {

// Holder for the action that is performed at the end of the scope. This class takes a callable
// object on creation and calls it on destruction.
template <typename Action>
class DeferAction : public SoFUtil::NoCopyMove {
public:
  explicit DeferAction(Action action) : action_(std::move(action)) {}
  ~DeferAction() { action_(); }

private:
  Action action_;
};

// A simple way to create `DeferAction`. Usage example:
//
// SOF_DEFER({
//   cout << "Scope has ended!" << endl;
// })
#define SOF_DEFER(...) \
  SoFUtil::DeferAction SOF_MAKE_UNIQUE(sofDeferActionHolder__)([&]() { __VA_ARGS__ });

}  // namespace SoFUtil

#endif  // SOF_UTIL_DEFER_INCLUDED
