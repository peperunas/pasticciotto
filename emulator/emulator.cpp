#include "../vm/debug.h"
#include "../vm/vm.h"
#include <fstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  std::ifstream bytecode_if;
  std::streamsize bytecode_size;
  uint8_t *bytecode;
  char test[] = "test";

  if (argc < 2) {
    printf("Usage: %s <opcodes_key> <program>\n", argv[0]);
    return 1;
  }

  /*
  reading bytecode
  */
  bytecode_if.open(argv[1], std::ios::binary | std::ios::ate);
  bytecode_size = bytecode_if.tellg();
  bytecode_if.seekg(0, std::ios::beg);

  bytecode = new uint8_t[bytecode_size];
  bytecode_if.read((char *)bytecode, bytecode_size);
  printf("SIZE READ: %d\n", bytecode_size);
  VM vm(test, bytecode, bytecode_size);
  vm.run();
  return 0;
}