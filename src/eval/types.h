// This file is part of SoFCheck
//
// Copyright (c) 2021 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_EVAL_TYPES_INCLUDED
#define SOF_EVAL_TYPES_INCLUDED

namespace SoFEval {

// Base struct to define helper types and methods for your score type (e. g. define a type to
// represent score pair). You must specialize this struct when you implement a score type.
template <typename T>
struct ScoreTraits {};

}  // namespace SoFEval

#endif  // SOF_EVAL_TYPES_INCLUDED
