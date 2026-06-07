#include "def.h"
extern void switch_to(struct Context *old_context, struct Context *new_context);
void uart_putc(char c) { UARTC = c; }
void print_string(const char *s) {
  while (*s) {
    uart_putc(*s);
    s++;
  }
}
void print_int(unsigned int cnt) {
  if (cnt == 0) {
    uart_putc('0');
    return;
  }
  char buf[13];
  int i = 0;
  while (cnt) {
    buf[i++] = cnt % 10;
    cnt /= 10;
  }
  while (i) {
    uart_putc('0' + buf[--i]);
  }
}

void delay(int times) {
  for (volatile int i = 0; i < times; i++) {
  }
}
struct Task task1, task2;
struct Task *current_task;
void task_func1() {
  int cnt = 0;
  w_csr(sstatus, r_csr(sstatus) | STATUS_SIE);
  while (1) {
    print_string("func1 is working!!! cnt :");
    print_int(cnt++);
    print_string("\n");
    delay(500000);
  }
}
void task_func2() {
  w_csr(sstatus, r_csr(sstatus) | STATUS_SIE);
  int cnt = 0;
  while (1) {
    print_string("func2 is working!!! cnt:");
    print_int(cnt++);
    print_string("\n");
    delay(500000);
  }
}
void task_init() {
  task1.ctx.ra = (unsigned int)task_func1;
  task2.ctx.ra = (unsigned int)task_func2;

  task1.ctx.sp = (unsigned int)(&task1.stack[4095]);
  task2.ctx.sp = (unsigned int)(&task2.stack[4095]);
  current_task = &task1;
}

void trap_handler() {

  unsigned int sepc = r_csr(sepc); // 中断sepc 指向下一条指令 异常指向错误的指令
  unsigned int cause = r_csr(scause);
  int is_interupt = (cause & CAUSE_INT_MASK) != 0;
  unsigned int cause_code = (cause & CAUSE_CODE_MASK);
  if (is_interupt) {
    print_string("received a interrupt !!!!\n");
    w_csr(sip, 0); // 关掉门铃
    struct Task *old_task = current_task;
    if (current_task == &task1) {
      current_task = &task2;
    } else
      current_task = &task1;
    switch_to(&old_task->ctx, &current_task->ctx);

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
  print_string("after trap_handler\n");

  task_init();
  struct Context dummy_context;
  switch_to(&dummy_context, &task1.ctx);
  while (1) {
    print_string("never reach main loop !!!\n");
  }
  return 0;
}
