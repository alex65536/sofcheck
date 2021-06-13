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

#ifndef SOF_SEARCH_PRIVATE_DIAGNOSTICS_INCLUDED
#define SOF_SEARCH_PRIVATE_DIAGNOSTICS_INCLUDED

#include "config.h"
#include "util/misc.h"

// If the diagnostics are enabled, then it expands to its arguments, otherwise it does nothing. This
// macro can be used to write the code that will be run only in diagnostics mode. Usage example:
//
// DIAGNOSTIC({
//   cout << "I run only when diagnostics are enabled!" << endl;
// })
#ifdef USE_SEARCH_DIAGNOSTICS
#define DIAGNOSTIC(...) __VA_ARGS__
#else
#define DIAGNOSTIC(...)
#endif

// Equivalents of `SOF_ASSERT` and `SOF_ASSERT_MSG`. The difference is that they do nothing if the
// diagnostics are not enabled.
#define DGN_ASSERT(...) DIAGNOSTIC(SOF_ASSERT(__VA_ARGS__);)
#define DGN_ASSERT_MSG(...) DIAGNOSTIC(SOF_ASSERT_MSG(__VA_ARGS__);)

#endif  // SOF_SEARCH_PRIVATE_DIAGNOSTICS_INCLUDED
