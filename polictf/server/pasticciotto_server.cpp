#include "../../vm/debug.h"
#include "../../vm/vm.h"
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define OPCODES_KEYLEN 15
#define CODESIZE 0x300
#define FLAG_LEN 30

unsigned char EN_DATASECTION[] = {
    0x8c, 0xea, 0xbe, 0xaa, 0xed, 0xa0, 0xd0, 0x6b, 0x99, 0x1c, 0x52, 0x25,
    0xb9, 0xe6, 0xd8, 0xff, 0xf9, 0xe9, 0x92, 0x7a, 0x1c, 0xc5, 0xc4, 0x7e,
    0x2a, 0xec, 0x67, 0x32, 0x86, 0xca, 0xff, 0xf8, 0x3c, 0x1c, 0x77, 0x42,
    0xe3, 0x20, 0x29, 0x4b, 0x34, 0x67, 0x4b, 0xc9, 0x9f, 0xa9, 0xf9, 0x0c,
    0x0f, 0x9b, 0x8a, 0x5b, 0x72, 0x64, 0xe5, 0xd8, 0x5c, 0x52, 0x58, 0x46,
    0xef, 0x36, 0x76, 0x87, 0xec, 0x1e, 0xfb, 0x5d, 0x42, 0x8e, 0xb7, 0x47};

unsigned int DATASECTIONLEN = 72;

void gen_random(uint8_t *s, const int len) {
  srand(time(NULL));
  static const char alphanum[] = "0123456789"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz";
  for (int i = 0; i < len; ++i) {
    s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  s[len] = 0;
}

int main(int argc, char *argv[]) {
  uint8_t *opcodes_key = new uint8_t[OPCODES_KEYLEN],
          *decdatasec = new uint8_t[DATASECTIONLEN],
          *flag = new uint8_t[FLAG_LEN];
  uint8_t *clientcode;
  uint8_t i;
  uint32_t clientcodesize, bytesread;
  FILE *datap, *flagp;

  gen_random(opcodes_key, OPCODES_KEYLEN);
  printf("Use this: \"%s\"\n", opcodes_key);
  printf("How much data are you sending me?\n");
  fflush(stdout);
  scanf("%d", &clientcodesize);
  printf("Go ahead then!\n");
  fflush(stdout);
  clientcode = new uint8_t[clientcodesize];
  bytesread = read(0, clientcode, clientcodesize);
  if (bytesread != clientcodesize) {
    printf("ERROR! Couldn't read everything!\n");
    fflush(stdout);
    exit(1);
  }
  VM vm(opcodes_key, clientcode, clientcodesize);
  vm.as.insData(EN_DATASECTION, DATASECTIONLEN);
  vm.run();

  datap = fopen("../res/decrypteddatasection.txt", "r");
  if (datap == NULL) {
    printf("Couldn't open decrypteddatasection.txt!\n");
    fflush(stdout);
    exit(1);
  }
  fscanf(datap, "%s", decdatasec);
  fclose(datap);

  for (i = 0; i < DATASECTIONLEN; i++) {
    DBG_INFO(("Checking data[%d]..\n", i));
    if (vm.as.data[i] != decdatasec[i]) {
      printf("Nope!\n");
      fflush(stdout);
      exit(1);
    }
  }

  flagp = fopen("../res/flag.txt", "r");
  if (flagp == NULL) {
    printf("Couldn't open flag.txt!\n");
    fflush(stdout);
    exit(1);
  }
  fscanf(flagp, "%s", flag);
  fclose(flagp);
  printf("Congratulations!\nThe flag is: %s\n", flag);
  fflush(stdout);
  return 0;
}