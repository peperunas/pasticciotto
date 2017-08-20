#include "../include/catch.hpp"
#include "../../vm/vm.h"
#include <cstring>

TEST_CASE("VMAddrSpace initialization", "[VMAS]") {
    uint32_t i;
    uint8_t code_test[] = "testing stuff... code!";
    uint32_t code_len = strlen((const char *) code_test);
    uint8_t stack_test[] = "testing stuff... stack!";
    uint32_t stack_len = strlen((const char *) stack_test);
    uint8_t data_test[] = "testing stuff... data!";
    uint32_t data_len = strlen((const char *) data_test);


/*
 * DEFAULT SIZE
 */
    VMAddrSpace vmas_def;

    REQUIRE(vmas_def.getCodesize() == DEFAULT_CODESIZE);
    REQUIRE(vmas_def.getDatasize() == DEFAULT_DATASIZE);
    REQUIRE(vmas_def.getStacksize() == DEFAULT_STACKSIZE);
    REQUIRE(vmas_def.getCode() != NULL);
    REQUIRE(vmas_def.getStack() != NULL);
    REQUIRE(vmas_def.getData() != NULL);

// Code has to be empty
    for (i = 0; i < DEFAULT_CODESIZE; i++) {
        REQUIRE(vmas_def.getCode()[i] == 0);
    }
// Data has to be empty
    for (i = 0; i < DEFAULT_DATASIZE; i++) {
        REQUIRE(vmas_def.getData()[i] == 0);
    }
// Stack has to be empty
    for (i = 0; i < DEFAULT_STACKSIZE; i++) {
        REQUIRE(vmas_def.getStack()[i] == 0);
    }

    vmas_def.insCode(code_test, code_len);
// Code has to match
    for (i = 0; i < DEFAULT_CODESIZE; i++) {
        if (i < code_len) {
            REQUIRE(vmas_def.getCode()[i] == code_test[i]);
        } else {
            REQUIRE(vmas_def.getCode()[i] == 0);
        }
    }
    vmas_def.insData(data_test, data_len);
// Code has to match
    for (i = 0; i < DEFAULT_DATASIZE; i++) {
        if (i < data_len) {
            REQUIRE(vmas_def.getData()[i] == data_test[i]);
        } else {
            REQUIRE(vmas_def.getData()[i] == 0);
        }
    }
    vmas_def.insStack(stack_test, stack_len);
// Code has to match
    for (i = 0; i < DEFAULT_STACKSIZE; i++) {
        if (i < stack_len) {
            REQUIRE(vmas_def.getStack()[i] == stack_test[i]);
        } else {
            REQUIRE(vmas_def.getStack()[i] == 0);
        }
    }
/*
 * CUSTOM SIZE
 */

    uint32_t cs = 0x1234, ss = 0x4321, ds = 0x4657;
    VMAddrSpace vmas_cus(ss, cs, ds);

    REQUIRE(vmas_cus.getCodesize() == cs);
    REQUIRE(vmas_cus.getDatasize() == ds);
    REQUIRE(vmas_cus.getStacksize() == ss);
    REQUIRE(vmas_cus.getCode() != NULL);
    REQUIRE(vmas_cus.getStack() != NULL);
    REQUIRE(vmas_cus.getData() != NULL);

// Code has to be empty
    for (i = 0; i < cs; i++) {
        REQUIRE(vmas_cus.getCode()[i] == 0);
    }
// Data has to be empty
    for (i = 0; i < ds; i++) {
        REQUIRE(vmas_cus.getData()[i] == 0);
    }
// Stack has to be empty
    for (i = 0; i < ss; i++) {
        REQUIRE(vmas_cus.getStack()[i] == 0);
    }

    vmas_cus.insCode(code_test, code_len);
// Code has to match
    for (i = 0; i < cs; i++) {
        if (i < code_len) {
            REQUIRE(vmas_cus.getCode()[i] == code_test[i]);
        } else {
            REQUIRE(vmas_cus.getCode()[i] == 0);
        }
    }
    vmas_cus.insData(data_test, data_len);
// Code has to match
    for (i = 0; i < ds; i++) {
        if (i < data_len) {
            REQUIRE(vmas_cus.getData()[i] == data_test[i]);
        } else {
            REQUIRE(vmas_cus.getData()[i] == 0);
        }
    }
    vmas_cus.insStack(stack_test, stack_len);
// Code has to match
    for (i = 0; i < ss; i++) {
        if (i < stack_len) {
            REQUIRE(vmas_cus.getStack()[i] == stack_test[i]);
        } else {
            REQUIRE(vmas_cus.getStack()[i] == 0);
        }
    }
}