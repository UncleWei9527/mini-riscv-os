extern int main(void);
static inline void w_mstatus(unsigned int x) {
  asm volatile("csrw mstatus,%0" : : "r"(x));
}
static inline unsigned r_mstatus() {
  unsigned int x;
  asm volatile("csrr %0,mstatus" : "=r"(x));
  return x;
}
static inline void w_mepc(unsigned int x) {
  asm volatile("csrw mepc,%0" : : "r"(x));
}
static inline void w_pmpaddr0(unsigned int x) {
  asm volatile("csrw pmpaddr0,%0" : : "r"(x));
}
static inline void w_pmpcfg0(unsigned int x) {
  asm volatile("csrw pmpcfg0,%0" : : "r"(x));
}
static inline void w_medeleg(unsigned int x) {
  asm volatile("csrw medeleg ,%0" : : "r"(x));
}
static inline void w_mideleg(unsigned int x) {
  asm volatile("csrw mideleg ,%0" : : "r"(x));
}
void start() {
  w_pmpaddr0(0xffffffff); // 设置界限地址为全部
  w_pmpcfg0(0xf);         // RWX 权限全开
  w_medeleg(0xffff);
  w_mideleg(0xffff);
  // M mode -> S Mode 修改mstatus
  unsigned int mstatus = r_mstatus();
  mstatus &= ~(3 << 11);
  mstatus |= (1 << 11);
  w_mstatus(mstatus);
  w_mepc((int)main);
  asm volatile("mret");
}
