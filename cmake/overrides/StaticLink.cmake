# This file is part of SoFCheck
#
# Copyright (c) 2022 Alexander Kernozhitsky and SoFCheck contributors
#
# SoFCheck is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SoFCheck is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

# Overrides CMake configuration to allow static linking. This is NOT needed to build SoFCheck
# itself, but may be useful to build its dependencies. For instance, it is used in CI builds.

# Set `/MT` instead of `/MD` for MSVC. This is similar to
# https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#how-can-i-build-my-msvc-application-with-a-static-runtime.
if(MSVC)
  set(CMAKE_C_FLAGS_DEBUG_INIT            "/MTd /Zi /Ob0 /Od /RTC1")
  set(CMAKE_C_FLAGS_MINSIZEREL_INIT       "/MT /O1 /Ob1 /DNDEBUG")
  set(CMAKE_C_FLAGS_RELEASE_INIT          "/MT /O2 /Ob2 /DNDEBUG")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT   "/MT /Zi /O2 /Ob1 /DNDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG_INIT          "/MTd /Zi /Ob0 /Od /RTC1")
  set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT     "/MT /O1 /Ob1 /DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE_INIT        "/MT /O2 /Ob2 /DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "/MT /Zi /O2 /Ob1 /DNDEBUG")
endif()

# Do not search for dynamic libraries
if(UNIX)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()
