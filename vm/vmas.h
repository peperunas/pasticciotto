#ifndef VMAS_H
#define VMAS_H

#include <stdint.h>

#define DEFAULT_STACKSIZE 0x100
#define DEFAULT_CODESIZE 0x300
#define DEFAULT_DATASIZE 0x100

class VMAddrSpace {
private:
    uint32_t stacksize, codesize, datasize;

public:
    VMAddrSpace();

    VMAddrSpace(uint32_t ss, uint32_t cs, uint32_t ds);

    uint8_t *stack, *code, *data;

    bool allocate(void);

    bool insStack(uint8_t *buf, uint32_t size);

    bool insCode(uint8_t *buf, uint32_t size);

    bool insData(uint8_t *buf, uint32_t size);
};

#endif