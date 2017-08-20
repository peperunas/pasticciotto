#include "../include/catch.hpp"
#include "../../vm/vm.h"
#include <cstring>


TEST_CASE("VM initialization", "[VM]") {
    uint8_t test[] = "testing stuff";
    uint32_t i;
    VM vm_nocode(test);

/*
 * VM WITHOUT CODE
 */

// Each register has to be zero when the VM is initialized
    for (i = 0; i < NUM_REGS; i++) {
        REQUIRE(vm_nocode.reg(i) == 0);
        REQUIRE_NOTHROW(vm_nocode.reg(i));
    }

// Accessing an invalid register throws an exception
    for (; i < 255; i++) {
        REQUIRE_THROWS(vm_nocode.reg(i));
    }

// Code has to be empty
    for (i = 0; i < vm_nocode.addressSpace()->getCodesize(); i++) {
        REQUIRE(vm_nocode.addressSpace()->getCode()[i] == 0);
    }

// Data has to be empty
    for (i = 0; i < vm_nocode.addressSpace()->getDatasize(); i++) {
        REQUIRE(vm_nocode.addressSpace()->getData()[i] == 0);
    }

// Stack has to be empty
    for (i = 0; i < vm_nocode.addressSpace()->getStacksize(); i++) {
        REQUIRE(vm_nocode.addressSpace()->getStack()[i] == 0);
    }

/*
 * VM WITH CODE
 */
    uint32_t test_len;
    test_len = strlen((const char *) test);
    VM vm_code(test, test, test_len);

// Each register has to be zero when the VM is initialized
    for (i = 0; i < NUM_REGS; i++) {
        REQUIRE(vm_code.reg(i) == 0);
        REQUIRE_NOTHROW(vm_code.reg(i));
    }

// Accessing an invalid register throws an exception
    for (; i < 255; i++) {
        REQUIRE_THROWS(vm_code.reg(i));
    }

// Code is intact
    for (i = 0; i < vm_code.addressSpace()->getCodesize(); i++) {
        if (i < test_len) {
            REQUIRE(vm_code.addressSpace()->getCode()[i] == test[i]);
        } else {
            REQUIRE(vm_code.addressSpace()->getCode()[i] == 0);
        }
    }

// Data has to be empty
    for (i = 0; i < vm_code.addressSpace()->getDatasize(); i++) {
        REQUIRE(vm_code.addressSpace()->getData()[i] == 0);
    }

// Stack has to be empty
    for (i = 0; i < vm_code.addressSpace()->getStacksize(); i++) {
        REQUIRE(vm_code.addressSpace()->getStack()[i] == 0);
    }

}

