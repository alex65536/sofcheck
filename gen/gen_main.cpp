#include <cstring>
#include <fstream>
#include <iostream>

void doGenerate(std::ostream &out);

int main(int argc, char **argv) {
  if (argc == 1) {
    doGenerate(std::cout);
    return 0;
  }
  if (argc == 2 && strcmp(argv[1], "-h") != 0 && strcmp(argv[1], "--help") != 0) {
    std::ofstream file(argv[1]);
    doGenerate(file);
    return 0;
  }
  std::cerr << "usage: " << argv[0] << " OUT_FILE" << std::endl;
  return 0;
}
