#ifndef SOF_UTIL_DEFER_INCLUDED
#define SOF_UTIL_DEFER_INCLUDED

#include <memory>

#include "util/misc.h"
#include "util/no_copy_move.h"

namespace SoFUtil {

// Holder for the action that is performed at the end of the scope. This class takes a callable
// object on creation and calls it on destruction.
template <typename Action>
class DeferAction : public SoFUtil::NoCopyMove {
public:
  explicit DeferAction(Action action) : action_(std::move(action)) {}
  ~DeferAction() { action_(); }

private:
  Action action_;
};

// A simple way to create `DeferAction`. Usage example:
//
// SOF_DEFER({
//   cout << "Scope has ended!" << endl;
// })
#define SOF_DEFER(...) \
  SoFUtil::DeferAction SOF_MAKE_UNIQUE(sofDeferActionHolder__)([&]() { __VA_ARGS__ });

}  // namespace SoFUtil

#endif  // SOF_UTIL_DEFER_INCLUDED
