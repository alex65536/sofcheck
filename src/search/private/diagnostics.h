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
