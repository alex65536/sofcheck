#include "init.h"

#include <atomic>

#include "core/private/magic.h"

namespace SoFCore {

void init() {
  static std::atomic_flag initialized;
  if (initialized.test_and_set()) {
    return;
  }
  Private::initMagic();
}

}  // namespace SoFCore
