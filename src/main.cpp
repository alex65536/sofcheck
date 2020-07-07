#include <iostream>

#include "core/types.h"
#include "core/private/near_attacks.h"
#include "core/board.h"

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  std::cout << sizeof(SoFCore::Board) << std::endl;
  std::cout << "Hello, world!" << std::endl;
  return 0;
}
