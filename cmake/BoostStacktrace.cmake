# This file is part of SoFCheck
#
# Copyright (c) 2020-2022 Alexander Kernozhitsky and SoFCheck contributors
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

# Detects `boost::stacktrace`. If it's present, then the following variables are set:
# - `USE_BOOST_STACKTRACE`
# - `BOOST_STACKTRACE_TARGET` is set to name of the detected `boost::stacktrace` backend

include_guard(GLOBAL)

set(BOOST_STACKTRACE_BACKENDS
  stacktrace_windbg_cached
  stacktrace_windbg
  stacktrace_backtrace
  stacktrace_addr2line
  stacktrace_basic
)
find_package(Boost 1.65.0 OPTIONAL_COMPONENTS ${BOOST_STACKTRACE_BACKENDS})
if(Boost_FOUND)
  foreach(backend ${BOOST_STACKTRACE_BACKENDS})
    if(boost_${backend}_FOUND)
      set(USE_BOOST_STACKTRACE ON)
      message(STATUS "Found boost::stacktrace backend: ${backend}")
      set(BOOST_STACKTRACE_TARGET Boost::${backend})
      break()
    endif()
  endforeach()
endif()
