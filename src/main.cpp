#include <iostream>

#include "core/types.h"
#include "core/board.h"
#include "core/init.h"

int main(int argc, char **argv) {
  SoFCore::init();
  (void)argc;
  (void)argv;
  std::cout << sizeof(SoFCore::Board) << std::endl;
  std::cout << "Hello, world!" << std::endl;
  return 0;
}
