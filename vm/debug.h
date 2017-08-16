#ifndef DBG_H
#define DBG_H

#include <stdio.h>

#if DBG
#define DBG_INFO(_x_)                                                          \
  do {                                                                         \
    printf("\t[*] ");                                                          \
    printf _x_;                                                                \
  } while (0)
#define DBG_WARN(_x_)                                                          \
  do {                                                                         \
    printf("[!] ");                                                            \
    printf _x_;                                                                \
  } while (0)
#define DBG_ERROR(_x_)                                                         \
  do {                                                                         \
    printf("[-] ");                                                            \
    printf _x_;                                                                \
  } while (0)
#define DBG_SUCC(_x_)                                                          \
  do {                                                                         \
    printf("[+] ");                                                            \
    printf _x_;                                                                \
  } while (0)
#else
#define DBG_INFO(_x_)
#define DBG_WARN(_x_)
#define DBG_ERROR(_x_)
#define DBG_SUCC(_x_)
#endif

#endif