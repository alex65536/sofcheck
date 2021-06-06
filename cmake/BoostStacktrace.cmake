# Detects `boost::stacktrace`. If it's present, then the following variables are set:
# - `USE_BOOST_STACKTRACE`
# - `BOOST_STACKTRACE_TARGET` is set to name of the detected `boost::stacktrace` backend

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
