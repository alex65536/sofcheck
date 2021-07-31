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

#ifndef SOF_CORE_INIT_INCLUDED
#define SOF_CORE_INIT_INCLUDED

namespace SoFCore {

// Initialization routine. It must be called at least once from the main thread before any other
// method in `SoFCore` is used. If this method is called after other methods in `SoFCore` are used,
// then the behavior is undefined
//
// It is disregarded to use this function during static initialization
void init();

}  // namespace SoFCore

#endif  // SOF_CORE_INIT_INCLUDED
