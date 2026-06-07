#pragma once
#define UART0 0x10000000L
#define UARTC (*(volatile unsigned char *)(UART0))
#define CLINT_BASE 0x2000000L
#define CLINT_MTIMECMP (CLINT_BASE + 0x4000) // 闹钟
#define CLINT_MTIME (CLINT_BASE + 0xBFF8)
#define INTERVAL 1000000 // 0.1s
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

// 特权位值
// mstatus/status
#define STATUS_SIE (1 << 1)  // S态中断
#define STATUS_MIE (1 << 3)  // M态中断
#define STATUS_SPIE (1 << 5) // S态前中断
#define STATUS_MPIE (1 << 7) // M态前中断
#define STATUS_SPP (1 << 8)  // S态异常前特权级 0：User 1:Supervisor
#define STATUS_MPP (3 << 11) // M态异常前特权级 00:User 01:Supervisor 11:Machine
#define STATUS_MPP_U (0 << 11)
#define STATUS_MPP_S (1 << 11)
#define STATUS_MPP_M (3 << 11)

// MIE SIE(中断分闸开关寄存器)
#define IE_SSIE (1 << 1) // S态软中断开关
#define IE_MSIE (1 << 3) // M态软中断开关
#define IE_STIE (1 << 5) // S态定时器中断
#define IE_MTIE (1 << 7) // M态定时器中断

// M/S  Cause
#define CAUSE_INT_MASK 0x80000000
#define CAUSE_CODE_MASK 0x7fffffff

// PMPCFG 物理内存权限
#define PMP_R (1 << 0)
#define PMP_W (1 << 1)
#define PMP_X (1 << 2)
#define PMP_A_TOR (1 << 3)

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
