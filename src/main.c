#define UART0 0x10000000L
#define UARTC (*(volatile unsigned char *)(UART0))
void uart_putc(char c) { UARTC = c; }
void print_string(const char *s) {
  while (*s) {
    uart_putc(*s);
    s++;
  }
}
static inline void w_stvec(unsigned int x) {
  asm volatile("csrw stvec,%0" : : "r"(x));
}
static inline unsigned int r_scause() {
  unsigned int x;
  asm volatile("csrr %0,scause" : "=r"(x));
  return x;
}
static inline unsigned int r_sepc() {
  unsigned int x;
  asm volatile("csrr %0,sepc" : "=r"(x));
  return x;
}
static inline void w_sepc(unsigned int x) {
  asm volatile("csrw sepc,%0 " : : "r"(x));
  return;
}
void trap_handler() {
  unsigned int sepc = r_sepc(); // 中断sepc 指向下一条指令 异常指向错误的指令
  unsigned int cause = r_scause();
  int is_interupt = (cause & 0x80000000) != 0;
  unsigned int cause_code = (cause & 0x7fffffff);
  if (is_interupt) {
    print_string("received a interrupt !!!!\n");
  } else {
    if (cause_code == 9) {
      print_string("system call from S-Mode \n");
    } else
      print_string("unknown Exception\n");
    sepc += 4;
    w_sepc(sepc);
  }
}
extern void trap_vector();
int main() {
  w_stvec((unsigned int)trap_vector);
  print_string("Hello RISCV OS!!!");
  asm volatile("ecall");
  print_string("after trap_handler\n");
  while (1) {
  }
  return 0;
}
