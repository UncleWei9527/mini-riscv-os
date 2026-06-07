#include "def.h"
extern void forkret();
void sys_print(const char *s) {
  __asm__ volatile("mv a7,%0\n"
                   "mv a0,%1\n"
                   "ecall\n"
                   :
                   : "r"(1), "r"(s)
                   : "a0", "a7"

  );
}
extern void switch_to(struct Context *old_context, struct Context *new_context);
int uart_lock = 0;
void spin_lock() {
  int old_val;
  while (1) {
    __asm__ volatile("amoswap.w.aq %0 ,%1 ,(%2)"
                     : "=r"(old_val)
                     : "r"(1), "r"(&uart_lock)
                     : "memory");
    if (old_val == 0) {
      break;
    }
  }
}
void spin_unlock() {
  __asm__ volatile("amoswap.w.rl x0,x0,(%0)" : : "r"(&uart_lock) : "memory");
}
struct Task task_pool[MAX_TASK];
struct Task *current_task;
void task_init() {
  for (int i = 0; i < MAX_TASK; i++) {
    task_pool[i].pid = 0;
  }
}
int create_task(void (*entry)()) {
  for (int i = 0; i < MAX_TASK; i++) {
    if (task_pool[i].state == 0) {
      task_pool[i].state = 1;
      task_pool[i].pid = i + 1;
      task_pool[i].ctx.ra = (unsigned int)forkret;
      task_pool[i].ctx.s1 = (unsigned int)entry;
      task_pool[i].ctx.sp = (unsigned int)(&task_pool[i].stack[4096]);
      return task_pool[i].pid;
    }
  }
  return -1;
}

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

void task_func1() {
  int cnt = 0;
  // w_csr(sstatus, 0);
  while (1) {
    sys_print("func[1] is working !!! \n");
    delay(50000);
  }
}

void task_func2() {
  int cnt = 0;
  while (1) {
    sys_print("func[2] is working !!! \n");
    delay(50000);
  }
}

void task_func3() {
  int cnt = 0;
  while (1) {
    sys_print("func[3] is working !!! \n");
    delay(50000);
  }
}

void trap_handler(struct TrapFrame *tf) {

  unsigned int sepc = r_csr(sepc); // 中断sepc 指向下一条指令 异常指向错误的指令
  unsigned int cause = r_csr(scause);
  int is_interupt = (cause & CAUSE_INT_MASK) != 0;
  unsigned int cause_code = (cause & CAUSE_CODE_MASK);
  if (is_interupt) {
    // print_string("received a interrupt !!!!\n");
    w_csr(sip, 0); // 关掉门铃
    struct Task *old_task = current_task;
    int current_id = current_task - task_pool;
    int next_id = current_id;
    while (1) {
      next_id = (next_id + 1) % MAX_TASK;

      if (task_pool[next_id].state == 1) {
        current_task = &task_pool[next_id];
        break;
      }
    }
    switch_to(&old_task->ctx, &current_task->ctx);

  } else {
    if (cause_code == 9) {
      print_string("system call from S-Mode \n");
    } else if (cause_code == 8) {
      unsigned int syscall_num = tf->a7;
      unsigned int arg0 = tf->a0;
      if (syscall_num == 1) {
        spin_lock();
        print_string("Syscall:");
        print_string((char *)arg0);
        spin_unlock();
      }
      tf->epc += 4;
    } else {
      print_string("\n[KERNEL] CPU Caught a Crime!!! cause code:");
      print_int(cause_code);
      print_string("\n");
      while (1) {
      }
    }
    w_csr(sepc, sepc);
  }
}
extern void trap_vector();

int main() {
  w_csr(stvec, (unsigned int)trap_vector);
  print_string("Hello RISCV OS!!!");
  print_string("after trap_handler\n");

  task_init();
  create_task(task_func1);
  create_task(task_func2);
  create_task(task_func3);
  current_task = &task_pool[0];
  struct Context dummy_context;
  switch_to(&dummy_context, &current_task->ctx);

  while (1) {
    print_string("never reach main loop !!!\n");
  }
}
