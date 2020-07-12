#include "core/init.h"

#include <atomic>

#include "core/private/magic.h"
#include "core/private/zobrist.h"

namespace SoFCore {

void init() {
  static std::atomic_flag initialized;
  if (initialized.test_and_set()) {
    return;
  }
  Private::initMagic();
  Private::initZobrist();
}

}  // namespace SoFCore
