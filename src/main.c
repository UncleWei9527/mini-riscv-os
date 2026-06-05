#include "def.h"
void uart_putc(char c) { UARTC = c; }
void print_string(const char *s) {
  while (*s) {
    uart_putc(*s);
    s++;
  }
}
void trap_handler() {

  unsigned int sepc = r_csr(sepc); // 中断sepc 指向下一条指令 异常指向错误的指令
  unsigned int cause = r_csr(scause);
  int is_interupt = (cause & 0x80000000) != 0;
  unsigned int cause_code = (cause & 0x7fffffff);
  if (is_interupt) {
    print_string("received a interrupt !!!!\n");
    w_csr(sip, 0); // 关掉门铃
  } else {
    if (cause_code == 9) {
      print_string("system call from S-Mode \n");
    } else
      print_string("unknown Exception\n");
    sepc += 4;
    w_csr(sepc, sepc);
  }
}
extern void trap_vector();
int main() {
  w_csr(stvec, (unsigned int)trap_vector);
  print_string("Hello RISCV OS!!!");
  asm volatile("ecall");
  print_string("after trap_handler\n");
  while (1) {
  }
  return 0;
}
