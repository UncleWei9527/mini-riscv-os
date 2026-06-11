#pragma once
#define NULL 0
#define UART0 0x10000000L
#define UARTC (*(volatile unsigned char *)(UART0))
#define CLINT_BASE 0x2000000L
#define CLINT_MTIMECMP (CLINT_BASE + 0x4000) // 闹钟
#define CLINT_MTIME (CLINT_BASE + 0xBFF8)
#define INTERVAL 100000 // 0.1s
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
struct TrapFrame {
  unsigned int kernel_sp; // 0: 保存内核栈的栈顶地址 (核心！)
  unsigned int epc;       // 4: 保存被打断时的 PC
  unsigned int x1_ra;     // 8
  unsigned int x2_sp;     // 12: 存放真正的用户态 SP
  unsigned int x3_gp;     // 16
  unsigned int x4_tp;     // 20
  unsigned int x5_t0;     // 24
  unsigned int x6_t1;     // 28
  unsigned int x7_t2;     // 32
  unsigned int x8_s0;     // 36
  unsigned int x9_s1;     // 40
  unsigned int x10_a0;    // 44
  unsigned int x11_a1;    // 48
  unsigned int x12_a2;    // 52
  unsigned int x13_a3;    // 56
  unsigned int x14_a4;    // 60
  unsigned int x15_a5;    // 64
  unsigned int x16_a6;    // 68
  unsigned int x17_a7;    // 72
  unsigned int x18_s2;    // 76
  unsigned int x19_s3;    // 80
  unsigned int x20_s4;    // 84
  unsigned int x21_s5;    // 88
  unsigned int x22_s6;    // 92
  unsigned int x23_s7;    // 96
  unsigned int x24_s8;    // 100
  unsigned int x25_s9;    // 104
  unsigned int x26_s10;   // 108
  unsigned int x27_s11;   // 112
  unsigned int x28_t3;    // 116
  unsigned int x29_t4;    // 120
  unsigned int x30_t5;    // 124
  unsigned int x31_t6;    // 128
};
#define MAX_TASK 10
#define UNUSED 0
#define RUNNABLE 1
#define RUNNING 2
#define ZOMBIE 3
struct Task {
  struct Task *parent;
  int exit_code;
  struct Context ctx;
  int state; // 0 空闲 1 运行就绪
  int pid;   // 进程id
  struct TrapFrame tf;
  char kernel_stack[4096];
  char user_stack[4096];
};

void spin_lock();
void spin_unlock();
void printf(const char *fmt, ...);
void sys_print(const char *s);
void *sys_malloc(int size);
void sys_free(void *ptr);
void *malloc(int size);
void free(void *ptr);
void malloc_init();
void sys_exit(int status);
void exit(int status);
int wait(int *status);
int sys_wait(int *status);
