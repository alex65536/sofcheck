#include "init.h"

#include <atomic>

#include "core/private/magic.h"

namespace SoFCore {

std::atomic_flag initialized;

void init() {
  if (initialized.test_and_set()) {
    return;
  }
  Private::initMagic();
}

}  // namespace SoFCore
