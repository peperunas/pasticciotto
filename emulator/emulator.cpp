#include "../vm/debug.h"
#include "../vm/vm.h"
#include <fstream>

int main(int argc, char *argv[]) {
    std::ifstream bytecode_if;
    std::streamsize bytecode_size;
    uint8_t *bytecode;

    if (argc < 3) {
        printf("Usage: %s <opcodes_key> <program>\n", argv[0]);
        return 1;
    }

    /*
    reading bytecode
    */
    bytecode_if.open(argv[2], std::ios::binary | std::ios::ate);
    bytecode_size = bytecode_if.tellg();
    bytecode_if.seekg(0, std::ios::beg);

    bytecode = new uint8_t[bytecode_size];
    bytecode_if.read((char *) bytecode, bytecode_size);
    VM vm((uint8_t *) argv[1], bytecode, bytecode_size);
    vm.run();
    return 0;
}