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

#ifndef SOF_UTIL_NO_COPY_MOVE_INCLUDED
#define SOF_UTIL_NO_COPY_MOVE_INCLUDED

namespace SoFUtil {

// Inherit from this class if you want to forbid copying the object
struct NoCopy {
  NoCopy() = default;
  NoCopy(const NoCopy &) = delete;
  NoCopy(NoCopy &&) noexcept = default;
  NoCopy &operator=(const NoCopy &) = delete;
  NoCopy &operator=(NoCopy &&) noexcept = default;
};

// Inherit from this class if you want to forbid copying and moving the object
struct NoCopyMove {
  NoCopyMove() = default;
  NoCopyMove(const NoCopyMove &) = delete;
  NoCopyMove(NoCopyMove &&) = delete;
  NoCopyMove &operator=(const NoCopyMove &) = delete;
  NoCopyMove &operator=(NoCopyMove &&) = delete;
};

}  // namespace SoFUtil

#endif  // SOF_UTIL_NO_COPY_MOVE_INCLUDED
