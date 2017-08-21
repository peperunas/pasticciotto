#ifndef VM_H
#define VM_H

#include "vmas.h"
#include <stdint.h>
#include "instruction.h"


enum regs {
    R0, R1, R2, R3, S0, S1, S2, S3, IP, RP, SP, NUM_REGS
};
typedef struct flags {
    uint8_t ZF : 1;
    uint8_t CF : 1;
} flags_t;

class VM {
private:
    typedef bool (VM::*FuncPointer)(void);

    typedef struct instruction {
        const char *name;
        uint8_t value;
        const uint8_t length;
        VM::FuncPointer exec;
        bool isJump;
    } instruction_t;

    uint16_t regs[0xb];
    flags_t flags;
    VMAddrSpace as;
#ifdef DBG
    instruction_t INSTR[NUM_OPS]{
            {"MOVI", 0, MOVI_SIZE, &VM::execMOVI, false},
            {"MOVR", 0, MOVR_SIZE, &VM::execMOVR, false},
            {"LODI", 0, LODI_SIZE, &VM::execLODI, false},
            {"LODR", 0, LODR_SIZE, &VM::execLODR, false},
            {"STRI", 0, STRI_SIZE, &VM::execSTRI, false},
            {"STRR", 0, STRR_SIZE, &VM::execSTRR, false},
            {"ADDI", 0, ADDI_SIZE, &VM::execADDI, false},
            {"ADDR", 0, ADDR_SIZE, &VM::execADDR, false},
            {"SUBI", 0, SUBI_SIZE, &VM::execSUBI, false},
            {"SUBR", 0, SUBR_SIZE, &VM::execSUBR, false},
            {"ANDB", 0, ANDB_SIZE, &VM::execANDB, false},
            {"ANDW", 0, ANDW_SIZE, &VM::execANDW, false},
            {"ANDR", 0, ANDR_SIZE, &VM::execANDR, false},
            {"YORB", 0, YORB_SIZE, &VM::execYORB, false},
            {"YORW", 0, YORW_SIZE, &VM::execYORW, false},
            {"YORR", 0, YORR_SIZE, &VM::execYORR, false},
            {"XORB", 0, XORB_SIZE, &VM::execXORB, false},
            {"XORW", 0, XORW_SIZE, &VM::execXORW, false},
            {"XORR", 0, XORR_SIZE, &VM::execXORR, false},
            {"NOTR", 0, NOTR_SIZE, &VM::execNOTR, false},
            {"MULI", 0, MULI_SIZE, &VM::execMULI, false},
            {"MULR", 0, MULR_SIZE, &VM::execMULR, false},
            {"DIVI", 0, DIVI_SIZE, &VM::execDIVI, false},
            {"DIVR", 0, DIVR_SIZE, &VM::execDIVR, false},
            {"SHLI", 0, SHLI_SIZE, &VM::execSHLI, false},
            {"SHLR", 0, SHLR_SIZE, &VM::execSHLR, false},
            {"SHRI", 0, SHRI_SIZE, &VM::execSHRI, false},
            {"SHRR", 0, SHRR_SIZE, &VM::execSHRR, false},
            {"PUSH", 0, PUSH_SIZE, &VM::execPUSH, false},
            {"POOP", 0, POOP_SIZE, &VM::execPOOP, false},
            {"CMPB", 0, CMPB_SIZE, &VM::execCMPB, false},
            {"CMPW", 0, CMPW_SIZE, &VM::execCMPW, false},
            {"CMPR", 0, CMPR_SIZE, &VM::execCMPR, false},
            {"JMPI", 0, JMPI_SIZE, &VM::execJMPI, true},
            {"JMPR", 0, JMPR_SIZE, &VM::execJMPR, true},
            {"JPAI", 0, JPAI_SIZE, &VM::execJPAI, true},
            {"JPAR", 0, JPAR_SIZE, &VM::execJPAR, true},
            {"JPBI", 0, JPBI_SIZE, &VM::execJPBI, true},
            {"JPBR", 0, JPBR_SIZE, &VM::execJPBR, true},
            {"JPEI", 0, JPEI_SIZE, &VM::execJPEI, true},
            {"JPER", 0, JPER_SIZE, &VM::execJPER, true},
            {"JPNI", 0, JPNI_SIZE, &VM::execJPNI, true},
            {"JPNR", 0, JPNR_SIZE, &VM::execJPNR, true},
            {"CALL", 0, CALL_SIZE, &VM::execCALL, true},
            {"RETN", 0, RETN_SIZE, &VM::execRETN, true},
            {"SHIT", 0, SHIT_SIZE, &VM::execSHIT, false},
            {"NOPE", 0, NOPE_SIZE, &VM::execNOPE, false},
            {"GRMN", 0, GRMN_SIZE, &VM::execGRMN, false},
            {"DEBG", 0, DEBG_SIZE, &VM::execDEBG, false}
    };
#else
    instruction_t INSTR[NUM_OPS]{
            {"MOVI", 0, MOVI_SIZE, &VM::execMOVI, false},
            {"MOVR", 0, MOVR_SIZE, &VM::execMOVR, false},
            {"LODI", 0, LODI_SIZE, &VM::execLODI, false},
            {"LODR", 0, LODR_SIZE, &VM::execLODR, false},
            {"STRI", 0, STRI_SIZE, &VM::execSTRI, false},
            {"STRR", 0, STRR_SIZE, &VM::execSTRR, false},
            {"ADDI", 0, ADDI_SIZE, &VM::execADDI, false},
            {"ADDR", 0, ADDR_SIZE, &VM::execADDR, false},
            {"SUBI", 0, SUBI_SIZE, &VM::execSUBI, false},
            {"SUBR", 0, SUBR_SIZE, &VM::execSUBR, false},
            {"ANDB", 0, ANDB_SIZE, &VM::execANDB, false},
            {"ANDW", 0, ANDW_SIZE, &VM::execANDW, false},
            {"ANDR", 0, ANDR_SIZE, &VM::execANDR, false},
            {"YORB", 0, YORB_SIZE, &VM::execYORB, false},
            {"YORW", 0, YORW_SIZE, &VM::execYORW, false},
            {"YORR", 0, YORR_SIZE, &VM::execYORR, false},
            {"XORB", 0, XORB_SIZE, &VM::execXORB, false},
            {"XORW", 0, XORW_SIZE, &VM::execXORW, false},
            {"XORR", 0, XORR_SIZE, &VM::execXORR, false},
            {"NOTR", 0, NOTR_SIZE, &VM::execNOTR, false},
            {"MULI", 0, MULI_SIZE, &VM::execMULI, false},
            {"MULR", 0, MULR_SIZE, &VM::execMULR, false},
            {"DIVI", 0, DIVI_SIZE, &VM::execDIVI, false},
            {"DIVR", 0, DIVR_SIZE, &VM::execDIVR, false},
            {"SHLI", 0, SHLI_SIZE, &VM::execSHLI, false},
            {"SHLR", 0, SHLR_SIZE, &VM::execSHLR, false},
            {"SHRI", 0, SHRI_SIZE, &VM::execSHRI, false},
            {"SHRR", 0, SHRR_SIZE, &VM::execSHRR, false},
            {"PUSH", 0, PUSH_SIZE, &VM::execPUSH, false},
            {"POOP", 0, POOP_SIZE, &VM::execPOOP, false},
            {"CMPB", 0, CMPB_SIZE, &VM::execCMPB, false},
            {"CMPW", 0, CMPW_SIZE, &VM::execCMPW, false},
            {"CMPR", 0, CMPR_SIZE, &VM::execCMPR, false},
            {"JMPI", 0, JMPI_SIZE, &VM::execJMPI, true},
            {"JMPR", 0, JMPR_SIZE, &VM::execJMPR, true},
            {"JPAI", 0, JPAI_SIZE, &VM::execJPAI, true},
            {"JPAR", 0, JPAR_SIZE, &VM::execJPAR, true},
            {"JPBI", 0, JPBI_SIZE, &VM::execJPBI, true},
            {"JPBR", 0, JPBR_SIZE, &VM::execJPBR, true},
            {"JPEI", 0, JPEI_SIZE, &VM::execJPEI, true},
            {"JPER", 0, JPER_SIZE, &VM::execJPER, true},
            {"JPNI", 0, JPNI_SIZE, &VM::execJPNI, true},
            {"JPNR", 0, JPNR_SIZE, &VM::execJPNR, true},
            {"CALL", 0, CALL_SIZE, &VM::execCALL, true},
            {"RETN", 0, RETN_SIZE, &VM::execRETN, true},
            {"SHIT", 0, SHIT_SIZE, &VM::execSHIT, false},
            {"NOPE", 0, NOPE_SIZE, &VM::execNOPE, false},
            {"GRMN", 0, GRMN_SIZE, &VM::execGRMN, false},
    };
#endif

    ////////////////////////
    // FUNCTIONS
    ///////////////////////
    void initVariables(void);

    void encryptOpcodes(uint8_t *key);

    bool isRegValid(uint8_t reg);

    template<typename T>
    bool isDivArgValid(T arg) {
        if (arg == 0) {
            return false;
        }
        return true;
    }

    /*
    DBG UTILS
    */
    const char *getRegName(uint8_t);

    /*
    IMPLEMENTATIONS
    */
    bool execMOVI(void);

    bool execMOVR(void);

    bool execLODI(void);

    bool execLODR(void);

    bool execSTRI(void);

    bool execSTRR(void);

    bool execADDI(void);

    bool execADDR(void);

    bool execSUBI(void);

    bool execSUBR(void);

    bool execANDB(void);

    bool execANDW(void);

    bool execANDR(void);

    bool execYORB(void);

    bool execYORW(void);

    bool execYORR(void);

    bool execXORB(void);

    bool execXORW(void);

    bool execXORR(void);

    bool execNOTR(void);

    bool execMULI(void);

    bool execMULR(void);

    bool execDIVI(void);

    bool execDIVR(void);

    bool execSHLI(void);

    bool execSHLR(void);

    bool execSHRI(void);

    bool execSHRR(void);

    bool execPUSH(void);

    bool execPOOP(void);

    bool execCMPB(void);

    bool execCMPW(void);

    bool execCMPR(void);

    bool execJMPI(void);

    bool execJMPR(void);

    bool execJPAI(void);

    bool execJPAR(void);

    bool execJPBI(void);

    bool execJPBR(void);

    bool execJPEI(void);

    bool execJPER(void);

    bool execJPNI(void);

    bool execJPNR(void);

    bool execCALL(void);

    bool execRETN(void);

    bool execGRMN(void);

    bool execSHIT(void);

    bool execNOPE(void);

    bool execDEBG(void);

public:
    VM(uint8_t *key);

    VM(uint8_t *key, uint8_t *code, uint32_t codesize);

    void status(void);

    void run();

    VMAddrSpace *addressSpace();

    uint16_t reg(uint8_t);
};


#endif