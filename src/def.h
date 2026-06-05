#pragma once
#define UART0 0x10000000L
#define UARTC (*(volatile unsigned char *)(UART0))
#define CLINT_BASE 0x2000000L
#define CLINT_MTIMECMP (CLINT_BASE + 0x4000) // 闹钟
#define CLINT_MTIME (CLINT_BASE + 0xBFF8)
#define INTERVAL 10000000 // 0.1s
#define r_csr(reg)                                                             \
  ({                                                                           \
    unsigned int tmp;                                                          \
    asm volatile("csrr %0," #reg : "=r"(tmp));                                 \
    tmp;                                                                       \
  })
#define w_csr(reg, val)                                                        \
  {                                                                            \
    asm volatile("csrw " #reg ",%0" : : "r"(val));                             \
  }
