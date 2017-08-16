#ifndef VM_H
#define VM_H
#include "vmas.h"
#include <stdint.h>

enum regs { R0, R1, R2, R3, S0, S1, S2, S3, IP, RP, SP, NUM_REGS };
typedef struct flags {
  uint8_t ZF : 1;
  uint8_t CF : 1;
} flags_t;

class VM {
private:
  ////////////////////////
  // FUNCTIONS
  ///////////////////////
  void initVariables(void);
  void encryptOpcodes(uint8_t *key);
  bool dstRegCheck(uint8_t reg);
  bool srcDstRegCheck(uint8_t dst, uint8_t src);

  /*
  DBG UTILS
  */
  uint8_t *getRegName(uint8_t);
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

public:
  VM(uint8_t *key);
  VM(uint8_t *key, uint8_t *code, uint32_t codesize);
  uint16_t regs[0xb];
  flags_t flags;
  VMAddrSpace as;
  void status(void);
  void run();
};

#endif