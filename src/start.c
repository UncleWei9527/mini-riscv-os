extern int main(void);
#include "def.h"
unsigned int timer_scratch[5];
extern void timervec();
void time_init() {
  timer_scratch[3] = CLINT_MTIMECMP;
  timer_scratch[4] = INTERVAL;
  w_csr(mscratch, (unsigned int)timer_scratch);
  w_csr(mtvec, (unsigned int)timervec);
  volatile unsigned int *mtime_low = (unsigned int *)CLINT_MTIME;
  volatile unsigned int *mtime_high = (unsigned int *)(CLINT_MTIME + 4);
  volatile unsigned int *mtime_cmp_low = (unsigned int *)CLINT_MTIMECMP;
  volatile unsigned int *mtime_cmp_high = (unsigned int *)(CLINT_MTIMECMP + 4);

  // 获取当前时间
  unsigned int hi, lo;
  do {
    hi = *mtime_high;
    lo = *mtime_low;
  } while (hi != *mtime_high);
  unsigned long long current_time = (((unsigned long long)hi) << 32) | lo;
  unsigned long long next_time = current_time + INTERVAL;

  *mtime_cmp_low = 0xffffffff;
  *mtime_cmp_high = (unsigned int)(next_time >> 32);
  *mtime_cmp_low = next_time;

  w_csr(mie, r_csr(mie) | (1 << 7) | (1 << 1));
  unsigned int mstatus = r_csr(mstatus);

  w_csr(mstatus, mstatus | (1 << 1));
}
void start() {
  w_csr(pmpaddr0, 0xffffffff);
  w_csr(pmpcfg0, 0xf);
  w_csr(medeleg, 0xffff);
  w_csr(mideleg, 0xffff);
  time_init();
  // M mode -> S Mode 修改mstatus
  unsigned int mstatus = r_csr(mstatus);
  mstatus &= ~(3 << 11);
  mstatus |= (1 << 11);
  w_csr(mstatus, mstatus);
  w_csr(mepc, (int)main);
  asm volatile("mret");
}
