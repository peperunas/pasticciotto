#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void encrypt(uint16_t *v) {
  uint16_t v0 = v[0], v1 = v[1], i;
  uint16_t k0 = 0x7065; // "pe"
  uint16_t k1 = 0x7065; // "pe"
  uint16_t k2 = 0x7275; // "ru"
  uint16_t k3 = 0x6e73; // "ns"
  for (i = 0; i < 128; i++) {
    v0 += ((v1 << 4) + k0) ^ (v1) ^ ((v1 >> 5) + k1);
    v1 += ((v0 << 4) + k2) ^ (v0) ^ ((v0 >> 5) + k3);
  }
  v[0] = v0;
  v[1] = v1;
}

int main(int argc, char *argv[]) {
  uint8_t *buf;
  uint32_t buflen, i;
  FILE *f;
  if (argc != 2) {
    printf("Usage: %s text_to_encrypt", argv[0]);
    exit(1);
  }
  buflen = strlen(argv[1]);
  buf = (uint8_t *)malloc(buflen);
  memcpy(buf, argv[1], buflen);

  for (i = 0; i < buflen && i + 2 * (sizeof(uint16_t)) <= buflen;
       i += sizeof(uint32_t)) {
    encrypt((uint16_t *)&buf[i]);
  }

  write(1, buf, buflen);
  return 0;
}