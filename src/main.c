#include "def.h"
extern void forkret();
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
// 专门用来打印内存地址的法医工具
void print_hex(unsigned int val) {
  print_string("0x");
  char buf[9];
  for (int i = 7; i >= 0; i--) {
    int digit = val & 0xF;
    buf[i] = digit < 10 ? '0' + digit : 'A' + digit - 10;
    val >>= 4;
  }
  buf[8] = '\0';
  print_string(buf);
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
  printf("%s\n", __FUNCTION__);
  while (1) {
     printf("task func working\n");
    int *arr = (int *)malloc(sizeof(int) * 3);
    arr[0]=cnt;
    
    arr[0] = cnt;
    arr[1] = cnt * 10;
    arr[2] = cnt * 100;
    printf("func[1] is working arr[0]:%d arr[1]:%d arr[2]:%d !!! \n", arr[0],
           arr[1], arr[2]);
    cnt++;
    // free(arr);
    delay(50000);
  }
}

void task_func2() {
  printf("%s\n", __FUNCTION__);
  int cnt = 0;
  while (1) {

    printf("func[2] is working !!! cnt:%d\n", cnt++);
    delay(50000);
  }
}

void task_func3() {
  int cnt = 0;
  printf("%s\n", __FUNCTION__);
  malloc(16);
  while (1) {
    //printf("func[2] is working !!! cnt:%d\n", cnt++);
    int *arr = (int *)malloc(sizeof(int) * 3);
    arr[0] = cnt;
    arr[1] = cnt * 10;
    arr[2] = cnt * 100;
    printf("func[3] is working arr[0]:%d arr[1]:%d arr[2]:%d !!! \n", arr[0],
           arr[1], arr[2]);
    cnt++;
    free(arr);
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
    // print_string("进程正在调度\n");
    switch_to(&old_task->ctx, &current_task->ctx);

  } else {
    if (cause_code == 9) {
      print_string("system call from S-Mode \n");

    } else if (cause_code == 8) {
      unsigned int syscall_num = tf->a7;
      unsigned int arg0 = tf->a0;
      // print_string("syscall_num:");
      // print_int(syscall_num);
      if (syscall_num == 1) {
        // spin_lock();
        print_string("Syscall:");
        print_string((char *)arg0);
         //spin_unlock();
      }
      // malloc
      else if (syscall_num == 2) {
        // spin_lock();

         void *ptr = sys_malloc((int)arg0);
        
        // // 🌟 探针：逼大管家喊出他到底分出了什么地址！
        // print_string(" | sys_malloc return ptr: ");
        // print_hex((unsigned int)ptr);
        // print_string("\n");

        tf->a0 = (unsigned int)ptr;
        // spin_unlock();
      }
      // free
      else if (syscall_num == 3) {
        // spin_lock();
        sys_free((void *)arg0);

        // spin_unlock();
      }
      tf->epc += 4;
    } else {
      unsigned int stval = r_csr(stval); 
      
      print_string("\n==================================\n");
      print_string("[KERNEL PANIC] CPU Caught a Crime!!!\n");
      print_string("Cause Code: "); print_int(cause_code); print_string("\n");
      
      print_string("Death PC (sepc): "); 
      print_hex(sepc); // 肇事指令地址
      print_string("\n");
      
      print_string("Bad Address (stval): "); 
      print_hex(stval); // 被破坏的非法内存地址
      print_string("\n==================================\n");
      
      while (1) {} // 保护现场，停机
    }
    w_csr(sepc, sepc);
  }
}
extern void trap_vector();

int main() {
  w_csr(stvec, (unsigned int)trap_vector);
  print_string("Hello RISCV OS!!!");
  print_string("after trap_handler\n");

  malloc_init();
  task_init();
  create_task(task_func1);
  // create_task(task_func2);
  create_task(task_func3);
  current_task = &task_pool[0];
  struct Context dummy_context;
  switch_to(&dummy_context, &current_task->ctx);

  while (1) {
    print_string("never reach main loop !!!\n");
  }
}
