#include "vmas.h"
#include "debug.h"
#include <stdlib.h>
#include <string.h>

VMAddrSpace::VMAddrSpace() {
    stack = NULL;
    code = NULL;
    data = NULL;
    stacksize = DEFAULT_STACKSIZE;
    codesize = DEFAULT_CODESIZE;
    datasize = DEFAULT_DATASIZE;
    allocate();
    return;
}

VMAddrSpace::VMAddrSpace(uint32_t ss, uint32_t cs, uint32_t ds) {
    stack = NULL;
    code = NULL;
    data = NULL;
    stacksize = ss;
    codesize = cs;
    datasize = ds;
    allocate();
    return;
}

VMAddrSpace::~VMAddrSpace() {
    if (stack) {
        delete[] stack;
    }
    if (code) {
        delete[] code;
    }
    if (data) {
        delete[] data;
    }
    return;
}

bool VMAddrSpace::allocate(void) {
    DBG_INFO(("Allocating sections...\n"));

    DBG_INFO(("\tcode...\n"));
    code = new uint8_t[codesize];
    DBG_INFO(("\tdata...\n"));
    data = new uint8_t[datasize];
    DBG_INFO(("\tstack...\n"));
    stack = new uint8_t[stacksize];

    if (code == NULL) {
        DBG_ERROR(("Couldn't allocate code section.\n"));
        return false;
    }
    if (data == NULL) {
        DBG_ERROR(("Couldn't allocate data section.\n"));
        return false;
    }
    if (stack == NULL) {
        DBG_ERROR(("Couldn't allocate stack section.\n"));
        return false;
    }

    memset(code, 0x0, codesize);
    memset(stack, 0x0, stacksize);
    memset(data, 0x0, datasize);
    DBG_SUCC(("Done!\n"));
    return true;
}

bool VMAddrSpace::insCode(uint8_t *buf, uint32_t size) {
    if (code) {
        if (size > codesize) {
            DBG_ERROR(("The injected code size is too big!\n"));
            return false;
        }
        DBG_INFO(("Copying buffer into code section.\n"));
        memcpy(code, buf, size);
    } else {
        DBG_ERROR(("Couldn't write into code section.\n"));
        return false;
    }
    return true;
}

bool VMAddrSpace::insStack(uint8_t *buf, uint32_t size) {
    if (stack) {
        if (size > stacksize) {
            DBG_ERROR(("The injected stack size is too big!\n"));
            return false;
        }
        DBG_INFO(("Copying buffer into stack section.\n"));
        memcpy(stack, buf, size);
    } else {
        DBG_ERROR(("Couldn't write into stack section.\n"));
        return false;
    }
    return true;
}

bool VMAddrSpace::insData(uint8_t *buf, uint32_t size) {
    if (data) {
        if (size > datasize) {
            DBG_ERROR(("The injected data size is too big!\n"));
            return false;
        }
        DBG_INFO(("Copying buffer into data section.\n"));
        memcpy(data, buf, size);
    } else {
        DBG_ERROR(("Couldn't write into data section.\n"));
        return false;
    }
    return true;
}

uint32_t VMAddrSpace::getStacksize() const {
    return stacksize;
}

uint32_t VMAddrSpace::getCodesize() const {
    return codesize;
}

uint32_t VMAddrSpace::getDatasize() const {
    return datasize;
}
