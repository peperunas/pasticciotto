#include "vm.h"
#include "debug.h"
#include "opcodes.h"
#include <string.h>

void VM::encryptOpcodes(uint8_t *key) {
    uint8_t arr[256];
    uint32_t i, j, tmp, keysize;
    keysize = strlen((char *) key);

    DBG_INFO(("Using key: %s\n", key));
    /*
    RC4 KSA! :-D
    */
    for (i = 0; i < 256; i++) {
        arr[i] = i;
    }
    j = 0;
    for (i = 0; i < 256; i++) {
        j = (j + arr[i] + key[i % keysize]) % 256;
        tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
    for (i = 0; i < NUM_OPS; i++) {
        OPS[i] = arr[i];
    }
#ifdef DBG
    //#TODO ASSEGNARE I NOMI AGLI OPCODES
    DBG_INFO(("~~~~~~~~~~\nOPCODES:\n"));
    for (i = 0; i < NUM_OPS; i++) {
      DBG_INFO(("0x%x: 0x%x\n", i, OPS[i]));
    }
    DBG_INFO(("~~~~~~~~~~\n"));
#endif
    return;
}

/*
DBG UTILS
*/

uint8_t *VM::getRegName(uint8_t regvalue) {
    uint8_t *buf = new uint8_t[2];
#ifdef DBG
    switch (regvalue) {
    case R0:
      memcpy(buf, "R0", 2);
      break;
    case R1:
      memcpy(buf, "R1", 2);
      break;
    case R2:
      memcpy(buf, "R2", 2);
      break;
    case R3:
      memcpy(buf, "R3", 2);
      break;
    case S0:
      memcpy(buf, "S0", 2);
      break;
    case S1:
      memcpy(buf, "S1", 2);
      break;
    case S2:
      memcpy(buf, "S2", 2);
      break;
    case S3:
      memcpy(buf, "S3", 2);
      break;
    case IP:
      memcpy(buf, "IP", 2);
      break;
    case RP:
      memcpy(buf, "RP", 2);
      break;
    case SP:
      memcpy(buf, "SP", 2);
      break;
    default:
      memcpy(buf, "??", 2);
      break;
    }
#endif
    return buf;
}

void VM::status(void) {
#ifdef DBG
    uint8_t i;
    DBG_SUCC(("VM Status:\n"));
    DBG_SUCC(("~~~~~~~~~~\n"));
    for (i = R0; i <= SP; i++) {
      DBG_INFO(("%s:\t0x%04x\n", getRegName(i), regs[i]));
    }
    DBG_INFO(("Flags: ZF = %d, CF = %d\n", flags.ZF, flags.CF));
    DBG_SUCC(("~~~~~~~~~~\n"));
#endif
    return;
}

/*
CONSTRUCTORS
*/
VM::VM(uint8_t *key) {
    DBG_SUCC(("Creating VM without code.\n"));
    initVariables();
    encryptOpcodes(key);
}

VM::VM(uint8_t *key, uint8_t *code, uint32_t codesize) {
    DBG_SUCC(("Creating VM with code.\n"));
    as.insCode(code, codesize);
    initVariables();
    encryptOpcodes(key);
}

void VM::initVariables(void) {
    uint8_t i;

    for (i = R0; i < NUM_REGS; i++) {
        this->regs[i] = 0;
    }
    for (i = 0; i < NUM_OPS; i++) {
        OPS[i] = i;
    }
    return;
}

bool VM::isRegValid(uint8_t reg) {
    // invalid register
    if (reg < 0 || reg > NUM_REGS) {
        DBG_ERROR(("Unknown register: 0x%x.\n", reg));
        return false;
    }
    if (reg == IP || reg == SP || reg == RP) {
        DBG_ERROR(("Can't modify %s.\n", getRegName(reg)));
        return false;
    }
    return true;
}


/*
INSTRUCTIONS IMPLEMENTATIONS
*/

bool VM::execMOVI(void) {
    /*
    MOVI R0, 0x2400 | R0 = 0x2400
    */
    uint8_t dst;
    uint16_t imm;
    dst = as.code[regs[IP] + 1];
    imm = *((uint16_t *) &as.code[regs[IP] + 2]);
    DBG_INFO(("MOVI %s, 0x%x\n", getRegName(dst), imm));
    if (!isRegValid(dst)) {
        return false;
    }
    regs[dst] = imm;
    return true;
}

bool VM::execMOVR(void) {
    /*
    MOVR R1, R0 -> R1 = R0
    ---------------------
    R1, R0 = 0x10 <- DST / SRC are nibbles!
    */
    uint8_t dst, src;
    dst = as.code[regs[IP] + 1] >> 4;
    src = as.code[regs[IP] + 1] & 0b00001111;
    DBG_INFO(("MOVR %s, %s\n", getRegName(dst), getRegName(src)));
    if (!isRegValid(src) || !isRegValid(dst)) {
        return false;
    }
    regs[dst] = regs[src];
    return true;
}

bool VM::execLODI(void) {
    /*
    LODI R0, 0x1000 -> R0 = data[0x1000]
    */
    uint8_t dst;
    uint16_t src;
    dst = as.code[regs[IP] + 1];
    src = *((uint16_t *) &as.code[regs[IP] + 2]);
    DBG_INFO(("LODI %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(dst)) {
        return false;
    }
    regs[dst] = *((uint16_t *) &as.data[src]);
    return true;
}

bool VM::execLODR(void) {
    /*
    LODR R1, R0 -> R1 = data[R0]
    */
    uint16_t dst;
    uint8_t src;
    dst = as.code[regs[IP] + 1] >> 4;
    src = as.code[regs[IP] + 1] & 0b00001111;
    DBG_INFO(("LODR %s, %s\n", getRegName(dst), getRegName(src)));
    if (!isRegValid(src) || !isRegValid(dst)) {
        return false;
    }
    regs[dst] = *((uint16_t *) &as.data[regs[src]]);
    return true;
}

bool VM::execSTRI(void) {
    /*
    STRI 0x1000, R0 -> data[0x1000] = R0
    */
    uint16_t dst;
    uint8_t src;
    dst = *((uint16_t *) &as.code[regs[IP] + 1]);
    src = as.code[regs[IP] + 3];
    DBG_INFO(("STRI 0x%x, %s\n", dst, getRegName(src)));
    if (!isRegValid(dst)) {
        return false;
    }
    *((uint16_t *) &as.data[dst]) = regs[src];
    return true;
}

bool VM::execSTRR(void) {
    /*
    STRR R1, R0 -> data[R1] = R0
    */
    uint8_t dst;
    uint8_t src;

    dst = as.code[regs[IP] + 1] >> 4;
    src = as.code[regs[IP] + 1] & 0b00001111;
    DBG_INFO(("STRR %s, %s\n", getRegName(dst), getRegName(src)));
    if (!isRegValid(src) || !isRegValid(dst)) {
        return false;
    }
    *((uint16_t *) &as.data[regs[dst]]) = regs[src];
    return true;
}

bool VM::execADDI(void) {
    /*
    ADDI R0, 0x2 -> R0 += 2
    */
    uint8_t dst;
    uint16_t src;

    dst = as.code[regs[IP] + 1];
    src = *((uint16_t *) &as.code[regs[IP] + 2]);
    DBG_INFO(("ADDI %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(dst)) {
        return false;
    }
    regs[dst] += src;
    return true;
}

bool VM::execADDR(void) {
    /*
    ADDR R0, R1 -> R0 += R1
    */
    uint8_t dst;
    uint8_t src;

    dst = as.code[regs[IP] + 1] >> 4;
    src = as.code[regs[IP] + 1] & 0b00001111;
    DBG_INFO(("ADDR %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(src) || !isRegValid(dst)) {
        return false;
    }
    regs[dst] += regs[src];
    return true;
}

bool VM::execSUBI(void) {
    /*
    SUBI R0, 0x2 -> R0 -= 2
    */
    uint8_t dst;
    uint16_t src;

    dst = as.code[regs[IP] + 1];
    src = *((uint16_t *) &as.code[regs[IP] + 2]);
    DBG_INFO(("SUBI %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(dst)) {
        return false;
    }
    regs[dst] -= src;
    return true;
}

bool VM::execSUBR(void) {
    /*
    SUBR R0, R1 -> R0 -= R1
    */
    uint8_t dst;
    uint8_t src;

    dst = as.code[regs[IP] + 1] >> 4;
    src = as.code[regs[IP] + 1] & 0b00001111;
    DBG_INFO(("SUBR %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(src) || !isRegValid(dst)) {
        return false;
    }
    regs[dst] -= regs[src];
    return true;
}

bool VM::execANDB(void) {
    /*
    ANDB R0, 0x2 -> R0 &= 0x02 or R0 &= [BYTE] 0x02 (low byte)
    */
    uint8_t dst;
    uint8_t src;

    dst = as.code[regs[IP] + 1];
    src = as.code[regs[IP] + 2];
    DBG_INFO(("ANDB %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(dst)) {
        return false;
    }
    regs[dst] &= src;
    return true;
}

bool VM::execANDW(void) {
    /*
    ANDW R0, 0x2 -> R0 &= 0x0002 or R0, ^= [WORD] 0x2
    */
    uint8_t dst;
    uint16_t src;

    dst = as.code[regs[IP] + 1];
    src = *((uint16_t *) &as.code[regs[IP] + 2]);
    DBG_INFO(("XORW %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(dst)) {
        return false;
    }
    regs[dst] &= src;
    return true;
}

bool VM::execANDR(void) {
    /*
    ANDR R0, R1 -> R0 ^= R1
    */
    uint8_t dst;
    uint8_t src;

    dst = as.code[regs[IP] + 1] >> 4;
    src = as.code[regs[IP] + 1] & 0b00001111;
    DBG_INFO(("ANDR %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(src) || !isRegValid(dst)) {
        return false;
    }
    regs[dst] &= regs[src];
    return true;
}

bool VM::execYORB(void) {
    /*
    YORB R0, 0x2 -> R0 |= 0x02 or R0 |= [BYTE] 0x02 (low byte)
    */
    uint8_t dst;
    uint8_t src;

    dst = as.code[regs[IP] + 1];
    src = as.code[regs[IP] + 2];
    DBG_INFO(("YORB %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(dst)) {
        return false;
    }
    regs[dst] |= src;
    return true;
}

bool VM::execYORW(void) {
    /*
    YORW R0, 0x2 -> R0 |= 0x0002 or R0, |= [WORD] 0x2
    */
    uint8_t dst;
    uint16_t src;

    dst = as.code[regs[IP] + 1];
    src = *((uint16_t *) &as.code[regs[IP] + 2]);
    DBG_INFO(("XORW %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(dst)) {
        return false;
    }
    regs[dst] |= src;
    return true;
}

bool VM::execYORR(void) {
    /*
    YORR R0, R1 -> R0 |= R1
    */
    uint8_t dst;
    uint8_t src;

    dst = as.code[regs[IP] + 1] >> 4;
    src = as.code[regs[IP] + 1] & 0b00001111;
    DBG_INFO(("XORR %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(src) || !isRegValid(dst)) {
        return false;
    }
    regs[dst] |= regs[src];
    return true;
}

bool VM::execXORB(void) {
    /*
    XORB R0, 0x2 -> R0 ^= 0x02 or R0 ^= [BYTE] 0x02 (low byte)
    */
    uint8_t dst;
    uint8_t src;

    dst = as.code[regs[IP] + 1];
    src = as.code[regs[IP] + 2];
    DBG_INFO(("XORB %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(dst)) {
        return false;
    }
    regs[dst] ^= src;
    return true;
}

bool VM::execXORW(void) {
    /*
    XORW R0, 0x2 -> R0 ^= 0x0002 or R0, ^= [WORD] 0x2
    */
    uint8_t dst;
    uint16_t src;

    dst = as.code[regs[IP] + 1];
    src = *((uint16_t *) &as.code[regs[IP] + 2]);
    DBG_INFO(("XORW %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(dst)) {
        return false;
    }
    regs[dst] ^= src;
    return true;
}

bool VM::execXORR(void) {
    /*
    XORR R0, R1 -> R0 ^= R1
    */
    uint8_t dst;
    uint8_t src;

    dst = as.code[regs[IP] + 1] >> 4;
    src = as.code[regs[IP] + 1] & 0b00001111;
    DBG_INFO(("XORR %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(src) || !isRegValid(dst)) {
        return false;
    }
    regs[dst] ^= regs[src];
    return true;
}

bool VM::execNOTR(void) {
    /*
    NOTR R0, R1 -> R0 = ~R1
    */
    uint8_t dst;
    uint8_t src;

    dst = as.code[regs[IP] + 1] >> 4;
    src = as.code[regs[IP] + 1] & 0b00001111;
    DBG_INFO(("NOTR %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(src) || !isRegValid(dst)) {
        return false;
    }
    regs[dst] = ~regs[src];
    return true;
}

bool VM::execMULI(void) {
    /*
    MULI R0, 0x2 | R0 *= 2
    */
    uint8_t dst;
    uint16_t src;

    dst = as.code[regs[IP] + 1];
    src = *((uint16_t *) &as.code[regs[IP] + 2]);
    DBG_INFO(("SUBI %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(dst)) {
        return false;
    }
    regs[dst] *= src;
    return true;
}

bool VM::execMULR(void) {
    /*
    MULR R0, R1 -> R0 *= R1
    */
    uint8_t dst;
    uint8_t src;

    dst = as.code[regs[IP] + 1] >> 4;
    src = as.code[regs[IP] + 1] & 0b00001111;
    DBG_INFO(("MULR %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(src) || !isRegValid(dst)) {
        return false;
    }
    regs[dst] *= regs[src];
    return true;
}

bool VM::execDIVI(void) {
    /*
    DIVI R0, 0x2 | R0 /= 2
    */
    uint8_t dst;
    uint16_t src;

    dst = as.code[regs[IP] + 1];
    src = *((uint16_t *) &as.code[regs[IP] + 2]);
    DBG_INFO(("DIVI %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(dst)) {
        return false;
    }
    regs[dst] /= src;
    return true;
}

bool VM::execDIVR(void) {
    /*
    DIVR R0, R1 -> R0 /= R1
    */
    uint8_t dst;
    uint8_t src;

    dst = as.code[regs[IP] + 1] >> 4;
    src = as.code[regs[IP] + 1] & 0b00001111;
    DBG_INFO(("DIVR %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(src) || !isRegValid(dst)) {
        return false;
    }
    regs[dst] /= regs[src];
    return true;
}

bool VM::execSHLI(void) {
    /*
    SHLI R0, 0x2 | R0 << 2
    */
    uint8_t dst;
    uint16_t src;

    dst = as.code[regs[IP] + 1];
    src = *((uint16_t *) &as.code[regs[IP] + 2]);
    DBG_INFO(("SHLI %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(dst)) {
        return false;
    }
    regs[dst] = regs[dst] << src;
    return true;
}

bool VM::execSHLR(void) {
    /*
    SHLR R0, R1 -> R0 << R1
    */
    uint8_t dst;
    uint8_t src;

    dst = as.code[regs[IP] + 1] >> 4;
    src = as.code[regs[IP] + 1] & 0b00001111;
    DBG_INFO(("SHLR %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(src) || !isRegValid(dst)) {
        return false;
    }
    regs[dst] = regs[dst] << regs[src];
    return true;
}

bool VM::execSHRI(void) {
    /*
    SHRI R0, 0x2 | R0 >> 2
    */
    uint8_t dst;
    uint16_t src;

    dst = as.code[regs[IP] + 1];
    src = *((uint16_t *) &as.code[regs[IP] + 2]);
    DBG_INFO(("SHRI %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(dst)) {
        return false;
    }
    regs[dst] = regs[dst] >> src;
    return true;
}

bool VM::execSHRR(void) {
    /*
    SHRR R0, R1 -> R0 >> R1
    */
    uint8_t dst;
    uint8_t src;

    dst = as.code[regs[IP] + 1] >> 4;
    src = as.code[regs[IP] + 1] & 0b00001111;
    DBG_INFO(("SHRR %s, 0x%x\n", getRegName(dst), src));
    if (!isRegValid(src) || !isRegValid(dst)) {
        return false;
    }
    regs[dst] = regs[dst] >> regs[src];
    return true;
}

bool VM::execPUSH(void) {
    uint8_t src;

    src = as.code[regs[IP] + 1];
    DBG_INFO(("PUSH %s\n", getRegName(src)));
    if (!isRegValid(src)) {
        return false;
    }
    if (regs[SP] + sizeof(uint16_t) > 0xffff) {
        DBG_ERROR(("Out of bound: stack is going above 0xFFFF!\n"));
        return false;
    }
    memcpy(&as.stack[regs[SP]], &regs[src], sizeof(uint16_t));
    regs[SP] += sizeof(uint16_t);
    return true;
}

bool VM::execPOOP(void) {
    uint8_t dst;

    dst = as.code[regs[IP] + 1];
    DBG_INFO(("POOP %s\n", getRegName(dst)));
    if (!isRegValid(dst)) {
        return false;
    }
    if (regs[SP] - sizeof(uint16_t) < 0) {
        DBG_ERROR(("Out of bound: stack is going below 0!\n"));
        return false;
    }
    regs[SP] -= sizeof(uint16_t);
    memcpy(&regs[dst], &as.stack[regs[SP]], sizeof(uint16_t));
    return true;
}

bool VM::execCMPB(void) {
    /*
    CMPB R0, 0x2 -> Compare immediate with lower half (BYTE) register
    */
    uint8_t reg;
    uint8_t imm;

    reg = as.code[regs[IP] + 1];
    imm = as.code[regs[IP] + 2];
    DBG_INFO(("CMPB %s, 0x%x\n", getRegName(reg), imm));
    if (!isRegValid(reg)) {
        return false;
    }
    if (*((uint8_t *) &regs[reg]) == imm) {
        flags.ZF = 1;
    } else {
        flags.ZF = 0;
    }
    if (*((uint8_t *) &regs[reg]) > imm) {
        flags.CF = 0;
    } else {
        flags.CF = 1;
    }
    return true;
}

bool VM::execCMPW(void) {
    /*
    CMPW R0, 0x2 -> Compare immediate with whole (WORD) register
    */
    uint8_t reg;
    uint16_t imm;

    reg = as.code[regs[IP] + 1];
    imm = *((uint16_t *) &as.code[regs[IP] + 2]);
    DBG_INFO(("CMPW %s, 0x%x\n", getRegName(reg), imm));
    if (!isRegValid(reg)) {
        return false;
    }
    if (regs[reg] == imm) {
        flags.ZF = 1;
    } else {
        flags.ZF = 0;
    }
    if (regs[reg] > imm) {
        flags.CF = 0;
    } else {
        flags.CF = 1;
    }
    return true;
}

bool VM::execCMPR(void) {
    /*
    CMPR R0, R1 -> Compares 2 registers
    */
    uint8_t r1;
    uint8_t r2;

    r1 = as.code[regs[IP] + 1] >> 4;
    r2 = as.code[regs[IP] + 1] & 0b00001111;
    DBG_INFO(("CMPR %s, %s\n", getRegName(r1), getRegName(r2)));
    if (!isRegValid(r1) || !isRegValid(r2)) {
        return false;
    }
    if (regs[r1] == regs[r2]) {
        flags.ZF = 1;
    } else {
        flags.ZF = 0;
    }
    if (regs[r1] > regs[r2]) {
        flags.CF = 0;
    } else {
        flags.CF = 1;
    }
    return true;
}

bool VM::execJMPI(void) {
    /*
    JMPI 0x2000 -> IP = 0x2000
    */
    uint16_t imm;

    imm = *(uint16_t *) &as.code[regs[IP] + 1];
    DBG_INFO(("JMPI 0x%x\n", imm));
    regs[IP] = imm;
    return true;
}

bool VM::execJMPR(void) {
    /*
    JMPR R0 -> IP = R0
    */
    uint8_t reg;

    reg = as.code[regs[IP] + 1];
    DBG_INFO(("JMPR %s = 0x%x\n", getRegName(reg), regs[reg]));
    if (!isRegValid(reg)) {
        return false;
    }
    regs[IP] = regs[reg];
    return true;
}

bool VM::execJPAI(void) {
    /*
    JPAI 0x2000 -> Jump to 0x2000 if above
    */
    uint16_t imm;

    imm = *(uint16_t *) &as.code[regs[IP] + 1];
    DBG_INFO(("JPAI 0x%x\n", imm));
    if (flags.CF == 0 && flags.ZF == 0) {
        regs[IP] = imm;
    } else {
        regs[IP] += JPAI_SIZE;
    }
    return true;
}

bool VM::execJPAR(void) {
    /*
    JPAR R0 -> Jump to [R0] if above
    */
    uint8_t reg;

    reg = as.code[regs[IP] + 1];
    DBG_INFO(("JPAR %s = 0x%x\n", getRegName(reg), regs[reg]));
    if (!isRegValid(reg)) {
        return false;
    }
    if (flags.CF == 0 && flags.ZF == 0) {
        regs[IP] = reg;
    } else {
        regs[IP] += JPAR_SIZE;
    }
    return true;
}

bool VM::execJPBI(void) {
    /*
    JPBI 0x2000 -> Jump to 0x2000 if below
    */
    uint16_t imm;

    imm = *(uint16_t *) &as.code[regs[IP] + 1];
    DBG_INFO(("JPBI 0x%x\n", imm));
    if (flags.CF == 1) {
        regs[IP] = imm;
    } else {
        regs[IP] += JPBI_SIZE;
    }
    return true;
}

bool VM::execJPBR(void) {
    /*
    JPBR R0 -> Jump to [R0] if below
    */
    uint8_t reg;

    reg = as.code[regs[IP] + 1];
    DBG_INFO(("JPBR %s = 0x%x\n", getRegName(reg), regs[reg]));
    if (!isRegValid(reg)) {
        return false;
    }
    if (flags.CF == 1) {
        regs[IP] = reg;
    } else {
        regs[IP] += JPBR_SIZE;
    }
    return true;
}

bool VM::execJPEI(void) {
    /*
    JPEI 0x2000 -> Jump to 0x2000 if equal
    */
    uint16_t imm;

    imm = *(uint16_t *) &as.code[regs[IP] + 1];
    DBG_INFO(("JPEI 0x%x\n", imm));
    if (flags.ZF == 1) {
        regs[IP] = imm;
    } else {
        regs[IP] += JPEI_SIZE;
    }
    return true;
}

bool VM::execJPER(void) {
    /*
    JPNR R0 -> Jump to [R0] if equal
    */
    uint8_t reg;

    reg = as.code[regs[IP] + 1];
    DBG_INFO(("JPER %s = 0x%x\n", getRegName(reg), regs[reg]));
    if (!isRegValid(reg)) {
        return false;
    }
    if (flags.ZF == 1) {
        regs[IP] = reg;
    } else {
        regs[IP] += JPER_SIZE;
    }
    return true;
}

bool VM::execJPNI(void) {
    /*
    JPEI 0x2000 -> Jump to 0x2000 if not equal
    */
    uint16_t imm;

    imm = *(uint16_t *) &as.code[regs[IP] + 1];
    DBG_INFO(("JPNI 0x%x\n", imm));
    if (flags.ZF == 0) {
        regs[IP] = imm;
    } else {
        regs[IP] += JPNI_SIZE;
    }
    return true;
}

bool VM::execJPNR(void) {
    /*
    JPER R0 -> Jump to [R0] if not equal
    */
    uint8_t reg;

    reg = as.code[regs[IP] + 1];
    DBG_INFO(("JPNR %s = 0x%x\n", getRegName(reg), regs[reg]));
    if (!isRegValid(reg)) {
        return false;
    }
    if (flags.ZF == 0) {
        regs[IP] = reg;
    } else {
        regs[IP] += JPNR_SIZE;
    }
    return true;
}

bool VM::execCALL(void) {
    /*
    CALL 0x1000 -> Jump to data[0x1000] and saves the RP onto the stack
    */
    uint16_t dst;

    dst = *((uint16_t *) &as.code[regs[IP] + 1]);
    DBG_INFO(("CALL 0x%x\n", dst));
    *((uint16_t *) &as.stack[regs[SP]]) = regs[IP] + 3;
    regs[SP] += sizeof(uint16_t);
    regs[IP] = dst;
    return false;
}

bool VM::execRETN(void) {
    /*
    RETN -> IP = RP , restores saved return IP
    */
    regs[SP] -= sizeof(uint16_t);
    regs[RP] = *((uint16_t *) &as.stack[regs[SP]]);
    DBG_INFO(("RETN 0x%x\n", regs[RP]));
    regs[IP] = regs[RP];
    return true;
}

bool VM::execGRMN(void) {
    uint8_t i;
    for (i = 0; i < NUM_REGS; i++) {
        if (i != IP && i != RP) {
            regs[i] = 0x4747;
        }
    }
    return true;
}

void VM::run(void) {
    uint8_t opcode;
    bool finished = false;
    bool ret;
    while (!finished) {
        opcode = (uint8_t) as.code[regs[IP]];
        if (opcode == OPS[MOVI]) {
            ret = execMOVI();
            if (ret) {
                regs[IP] += MOVI_SIZE;
            } else {
                DBG_ERROR(("MOVI FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[MOVR]) {
            ret = execMOVR();
            if (ret) {
                regs[IP] += MOVR_SIZE;
            } else {
                DBG_ERROR(("MOVR FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[LODI]) {
            ret = execLODI();
            if (ret) {
                regs[IP] += LODI_SIZE;
            } else {
                DBG_ERROR(("LODI FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[LODR]) {
            ret = execLODR();
            if (ret) {
                regs[IP] += LODR_SIZE;
            } else {
                DBG_ERROR(("LODR FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[STRI]) {
            ret = execSTRI();
            if (ret) {
                regs[IP] += STRI_SIZE;
            } else {
                DBG_ERROR(("MOVI FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[STRR]) {
            ret = execSTRR();
            if (ret) {
                regs[IP] += STRR_SIZE;
            } else {
                DBG_ERROR(("STRR FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[ADDI]) {
            ret = execADDI();
            if (ret) {
                regs[IP] += ADDI_SIZE;
            } else {
                DBG_ERROR(("ADDI FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[ADDR]) {
            ret = execADDR();
            if (ret) {
                regs[IP] += ADDR_SIZE;
            } else {
                DBG_ERROR(("ADDR FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[SUBI]) {
            ret = execSUBI();
            if (ret) {
                regs[IP] += SUBI_SIZE;
            } else {
                DBG_ERROR(("SUBI FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[SUBR]) {
            ret = execSUBR();
            if (ret) {
                regs[IP] += SUBR_SIZE;
            } else {
                DBG_ERROR(("SUBR FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[ANDB]) {
            ret = execANDB();
            if (ret) {
                regs[IP] += ANDB_SIZE;
            } else {
                DBG_ERROR(("ANDB FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[ANDW]) {
            ret = execANDW();
            if (ret) {
                regs[IP] += ANDW_SIZE;
            } else {
                DBG_ERROR(("ANDW FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[ANDR]) {
            ret = execANDR();
            if (ret) {
                regs[IP] += ANDR_SIZE;
            } else {
                DBG_ERROR(("ANDR FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[YORB]) {
            ret = execYORB();
            if (ret) {
                regs[IP] += YORB_SIZE;
            } else {
                DBG_ERROR(("YORB FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[YORW]) {
            ret = execYORW();
            if (ret) {
                regs[IP] += YORW_SIZE;
            } else {
                DBG_ERROR(("YORW FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[YORR]) {
            ret = execYORR();
            if (ret) {
                regs[IP] += YORR_SIZE;
            } else {
                DBG_ERROR(("YORR FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[XORB]) {
            ret = execXORB();
            if (ret) {
                regs[IP] += XORB_SIZE;
            } else {
                DBG_ERROR(("XORB FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[XORW]) {
            ret = execXORW();
            if (ret) {
                regs[IP] += XORW_SIZE;
            } else {
                DBG_ERROR(("XORW FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[XORR]) {
            ret = execXORR();
            if (ret) {
                regs[IP] += XORR_SIZE;
            } else {
                DBG_ERROR(("XORR FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[NOTR]) {
            ret = execNOTR();
            if (ret) {
                regs[IP] += NOTR_SIZE;
            } else {
                DBG_ERROR(("NOTR FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[MULI]) {
            ret = execMULI();
            if (ret) {
                regs[IP] += MULI_SIZE;
            } else {
                DBG_ERROR(("MULI FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[MULR]) {
            ret = execMULR();
            if (ret) {
                regs[IP] += MULR_SIZE;
            } else {
                DBG_ERROR(("MULR FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[DIVI]) {
            ret = execDIVI();
            if (ret) {
                regs[IP] += DIVI_SIZE;
            } else {
                DBG_ERROR(("DIVI FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[DIVR]) {
            ret = execDIVR();
            if (ret) {
                regs[IP] += DIVR_SIZE;
            } else {
                DBG_ERROR(("DIVR FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[SHLI]) {
            ret = execSHLI();
            if (ret) {
                regs[IP] += SHLI_SIZE;
            } else {
                DBG_ERROR(("SHLI FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[SHLR]) {
            ret = execSHLR();
            if (ret) {
                regs[IP] += SHLR_SIZE;
            } else {
                DBG_ERROR(("SHLR FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[SHRI]) {
            ret = execSHRI();
            if (ret) {
                regs[IP] += SHRI_SIZE;
            } else {
                DBG_ERROR(("SHRI FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[SHRR]) {
            ret = execSHRR();
            if (ret) {
                regs[IP] += SHRR_SIZE;
            } else {
                DBG_ERROR(("SHRR FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[PUSH]) {
            ret = execPUSH();
            if (ret) {
                regs[IP] += PUSH_SIZE;
            } else {
                DBG_ERROR(("PUSH FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[POOP]) {
            ret = execPOOP();
            if (ret) {
                regs[IP] += POOP_SIZE;
            } else {
                DBG_ERROR(("POOP FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[CMPB]) {
            ret = execCMPB();
            if (ret) {
                regs[IP] += CMPB_SIZE;
            } else {
                DBG_ERROR(("CMPB FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[CMPW]) {
            ret = execCMPW();
            if (ret) {
                regs[IP] += CMPW_SIZE;
            } else {
                DBG_ERROR(("CMPW FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[CMPR]) {
            ret = execCMPR();
            if (ret) {
                regs[IP] += CMPR_SIZE;
            } else {
                DBG_ERROR(("CMPR FAILED.\n"));
                finished = true;
            }
        } else if (opcode == OPS[JMPI]) {
            if (!execJMPI()) {
                finished = true;
            }
        } else if (opcode == OPS[JMPR]) {
            if (!execJMPR()) {
                finished = true;
            }
        } else if (opcode == OPS[JPAI]) {
            if (!execJPAI()) {
                finished = true;
            }
        } else if (opcode == OPS[JPAR]) {
            if (!execJPAR()) {
                finished = true;
            }
        } else if (opcode == OPS[JPBI]) {
            if (!execJPBI()) {
                finished = true;
            }
        } else if (opcode == OPS[JPBR]) {
            if (!execJPBR()) {
                finished = true;
            }
        } else if (opcode == OPS[JPEI]) {
            if (!execJPEI()) {
                finished = true;
            }
        } else if (opcode == OPS[JPER]) {
            if (!execJPER()) {
                finished = true;
            }
        } else if (opcode == OPS[JPNI]) {
            if (!execJPNI()) {
                finished = true;
            }
        } else if (opcode == OPS[JPNR]) {
            if (!execJPNR()) {
                finished = true;
            }
        } else if (opcode == OPS[CALL]) {
            execCALL();
        } else if (opcode == OPS[RETN]) {
            execRETN();
            regs[IP] = regs[RP];
        } else if (opcode == OPS[GRMN]) {
            execGRMN();
            regs[IP] += GRMN_SIZE;
        } else if (opcode == OPS[SHIT]) {
            DBG_INFO(("Halting.\n"));
            finished = true;
        } else if (opcode == OPS[NOPE]) {
            regs[IP] += NOPE_SIZE;
        }
#ifdef DBG
            else if (opcode == OPS[DEBG]) {
              status();
              regs[IP] += DEBG_SIZE;
            }
#endif
        else {
            DBG_ERROR(("WAT: 0x%x\n", as.code[regs[IP]]));
            finished = true;
        }
    }
    DBG_INFO(("Finished.\n"));
    return;
}