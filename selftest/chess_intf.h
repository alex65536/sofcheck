// This file is part of SoFCheck
//
// Copyright (c) 2020-2021 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SELFTEST_CHESS_INTF_INCLUDED
#define SELFTEST_CHESS_INTF_INCLUDED

#ifdef INTF_DODECAHEDRON
#ifdef INTF_USED
#error Interface is already included!
#endif
#define INTF_USED
#include "dodecahedron/intf.h"
#endif

#ifdef INTF_SOFCHECK
#ifdef INTF_USED
#error Interface is already included!
#endif
#define INTF_USED
#include "sofcheck/intf.h"
#endif

#ifndef INTF_USED
#error No interface included!
#endif

#endif  // SELFTEST_CHESS_INTF_INCLUDED
