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

#ifndef SOF_UTIL_PARALLEL_INCLUDED
#define SOF_UTIL_PARALLEL_INCLUDED

#include <functional>

namespace SoFUtil {

// Given a segment `[left; right)`, splits it onto subsegments and processes these subsegments in
// parallel. Maximum allowed number of threads is `jobs`. `func` is a function that takes two
// arguments (`l` and `r`) and processes the subsegment `[l; r)`.
void processSegmentParallel(size_t left, size_t right, size_t jobs,
                            const std::function<void(size_t, size_t)> &func);

}  // namespace SoFUtil

#endif  // SOF_UTIL_PARALLEL_INCLUDED
