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
