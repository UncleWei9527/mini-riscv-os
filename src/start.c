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
  // 开启定时器中断
  w_csr(mie, r_csr(mie) | IE_MTIE | IE_SSIE);

  // w_csr(mstatus, r_csr(mstatus) | STATUS_MIE);
}
extern char __bss_end[];
extern char __bss_start[];
void bss_init()
{
  for (char *p = __bss_start; p < __bss_end; p++) {
    *p = 0;
}
}
void start() {
  w_csr(pmpaddr0, 0xffffffff);
  w_csr(pmpcfg0, PMP_R | PMP_W | PMP_X | PMP_A_TOR);
  w_csr(medeleg, 0xffff);
  w_csr(mideleg, 0xffff);
  time_init();
  // M mode -> S Mode 修改mstatus
  unsigned int mstatus = r_csr(mstatus);
  mstatus &= ~STATUS_MPP;
  mstatus |= STATUS_MPP_S;
  mstatus |= STATUS_SPIE;
  w_csr(mstatus, mstatus);
  w_csr(mepc, (int)main);
  asm volatile("mret");
}
