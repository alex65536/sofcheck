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

include(Platform)

set(BOOST_STACKTRACE_BACKENDS
  stacktrace_windbg_cached
  stacktrace_windbg
  stacktrace_backtrace
  stacktrace_addr2line
  stacktrace_basic
)

if(USE_STATIC_LINK)
  set(Boost_USE_STATIC_LIBS ON)
endif()

find_package(Boost 1.65.0 OPTIONAL_COMPONENTS ${BOOST_STACKTRACE_BACKENDS})

set(USE_BOOST_STACKTRACE OFF)
set(BOOST_STACKTRACE_TARGET)

if(Boost_FOUND)
  foreach(backend ${BOOST_STACKTRACE_BACKENDS})
    if(boost_${backend}_FOUND)
      message(STATUS "Found boost::stacktrace backend: ${backend}")
      if(USE_NO_EXCEPTIONS)
        message(WARNING
          "USE_NO_EXCEPTIONS conflicts with USE_BOOST_STACKTRACE. Disabling boost::stacktrace support.")
        break()
      endif()
      if(USE_STATIC_LINK)
        if("${backend}" MATCHES "^stacktrace_(basic|addr2line)$")
          target_link_libraries(Boost::${backend} INTERFACE dl)
        elseif("${backend}" STREQUAL stacktrace_backtrace)
          target_link_libraries(Boost::${backend} INTERFACE dl backtrace)
        endif()
      endif()
      set(USE_BOOST_STACKTRACE ON)
      set(BOOST_STACKTRACE_TARGET Boost::${backend})
      break()
    endif()
  endforeach()
endif()
