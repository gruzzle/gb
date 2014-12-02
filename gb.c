#include "gb.h"

int main(int argc, const char* argv[]) {
  initializeMemory();
  initializeCPU();
  mainLoop();
}
