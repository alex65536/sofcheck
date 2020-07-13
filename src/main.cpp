#include <iostream>

#include "core/board.h"
#include "core/init.h"
#include "core/types.h"

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  SoFCore::init();
  SoFCore::Board board =
      SoFCore::Board::fromFen("r1bqkbnr/1ppp1ppp/p1n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 4")
          .unwrap();
  std::cout << board.asPretty() << std::endl;
  return 0;
}
