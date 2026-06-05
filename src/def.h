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
struct Context {
  unsigned int ra;
  unsigned int sp;

  unsigned int s0;
  unsigned int s1;
  unsigned int s2;
  unsigned int s3;
  unsigned int s4;
  unsigned int s5;
  unsigned int s6;
  unsigned int s7;
  unsigned int s8;
  unsigned int s9;
  unsigned int s10;
  unsigned int s11;
};
struct Task {
  struct Context ctx;
  // int state; // 0 空闲 1 运行就绪
  char stack[4096];
};
