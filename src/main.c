#include "def.h"
extern void trap_return_asm(struct TrapFrame*tf);
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
//S Mode->U Mode
void forkret()
{
  unsigned int sstatus = r_csr(sstatus);
  sstatus &= ~STATUS_SPP; 
  w_csr(sstatus, sstatus);
  
  // 借助汇编跳入用户态
  trap_return_asm(&current_task->tf);
}
int create_task(void (*entry)()) {
  for (int i = 0; i < MAX_TASK; i++) {
    if (task_pool[i].state == UNUSED) {
      task_pool[i].state = RUNNABLE;
      task_pool[i].pid = i + 1;
     // 使用 & ~15 抹平最后 4 个二进制位，强行 16 字节对齐！保命神技！
      task_pool[i].tf.epc = (unsigned int)entry; 
      task_pool[i].tf.x2_sp = ((unsigned int)(&task_pool[i].user_stack[4096])) & ~15; 
      task_pool[i].tf.kernel_sp = ((unsigned int)(&task_pool[i].kernel_stack[4096])) & ~15; 
      
      task_pool[i].ctx.ra = (unsigned int)forkret; 
      task_pool[i].ctx.sp = ((unsigned int)(&task_pool[i].kernel_stack[4096])) & ~15;
      
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
  int i=0;
  while (i++<20) {
    int *arr = (int *)malloc(sizeof(int) * 3);
    arr[0]=cnt;
    
    arr[0] = cnt;
    arr[1] = cnt * 10;
    arr[2] = cnt * 100;
    printf("func[1] is working arr[0]:%d arr[1]:%d arr[2]:%d !!! \n", arr[0],
           arr[1], arr[2]);
    cnt++;
    free(arr);
    delay(50000);
  }
  exit(10);
}

void task_func2() {
  printf("%s\n", __FUNCTION__);
  int cnt = 0;
  int i=0;
  while (i++<10) {

    printf("func[2] is working !!! cnt:%d\n", cnt++);
    delay(50000);
  }
  exit(233);
}

void task_func3() {
 // asm volatile("li sp, 0");
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
void exit(int status)
{
    register unsigned int a7 asm("a7") = 4;
    register unsigned int a0 asm("a0") = status;

    __asm__ volatile("ecall" : : "r"(a7), "r"(a0) : "memory");
}
void sys_exit(int status)
{
  struct Context*old_ctx=&current_task->ctx;
  current_task->exit_code=status;
  current_task->state=ZOMBIE;
  //查找下个runnable 进程
  int current_id = current_task - task_pool;
  int next_id = current_id;
  print_string("TASK: ");
  print_int(current_id);
  print_string(" exit with ");
  print_int( status);
  print_string("\n");
  int loop_count=0;
  while (1)
  {
    next_id = (next_id + 1) % MAX_TASK;
    if (task_pool[next_id].state == RUNNABLE)
    {
      current_task = &task_pool[next_id];
      current_task->state = RUNNING;
      break;
    }
    loop_count++;
    if (loop_count >= MAX_TASK) {
      print_string("\n[KERNEL PANIC] No runnable tasks! System HALT!\n");
      while(1); // 停机
    }
  }
  switch_to(old_ctx,&current_task->ctx);

}
void trap_handler() { 
  struct TrapFrame *tf = &current_task->tf; // 直接拿结构体
  
  unsigned int cause = r_csr(scause);
  int is_interupt = (cause & CAUSE_INT_MASK) != 0;
  unsigned int cause_code = (cause & CAUSE_CODE_MASK);

  if (is_interupt) {
    w_csr(sip, 0); 
    struct Task *old_task = current_task;
    int current_id = current_task - task_pool;
    int next_id = current_id;
    current_task->state=RUNNABLE;
    int loop_count=0;
    while (1) {
      next_id = (next_id + 1) % MAX_TASK;
      if (task_pool[next_id].state == RUNNABLE) {

        current_task = &task_pool[next_id];
        current_task->state=RUNNING;
        break;
      }
      loop_count++;
      if (loop_count >= MAX_TASK)
      {
        print_string("\n[KERNEL PANIC] No runnable tasks! System HALT!\n");
        while (1)
          ; // 停机
      }
    }
    // 任务切换
    switch_to(&old_task->ctx, &current_task->ctx);
  } else {
    // 处理异常
    if (cause_code == 8) { 
      tf->epc += 4;  // 【注意】这里不再是修改 tf->epc，因为我们从 tf 里拿
      
      unsigned int syscall_num = tf->x17_a7;
      unsigned int arg0 = tf->x10_a0;
      //printf
      if (syscall_num == 1) {
        print_string("Syscall:");
        print_string((char *)arg0); 
      }
      //malloc
      else if (syscall_num == 2) {
        void *ptr = sys_malloc((int)arg0);
        tf->x10_a0 = (unsigned int)ptr; // 把返回值写回 tf
      }
      //free
      else if(syscall_num==3)
      {
        void*ptr=(void*)tf->x10_a0;
        sys_free(ptr);
      }
      //exit 
      else if(syscall_num==4)
      {
        int exit_code=(int)tf->x10_a0;
        sys_exit(exit_code);
      }
      // free ...
    } else {
       print_string("exception:");
       print_int(cause_code);
       while(1)
       {

       }
    }
  }

  // 万剑归宗！无论是系统调用完，还是 switch_to 切回来的进程，统统从这里返回用户态！
  trap_return_asm(tf); 
}
extern void trap_vector();

int main() {
  w_csr(stvec, (unsigned int)trap_vector);
  print_string("Hello RISCV OS!!!");
  print_string("after trap_handler\n");

  malloc_init();
  create_task(task_func1);
  create_task(task_func2);
  //create_task(task_func3);
  current_task = &task_pool[0];
  struct Context dummy_context;
  switch_to(&dummy_context, &current_task->ctx);

  while (1) {
    print_string("never reach main loop !!!\n");
  }
}
//switch_to get context ra->forkret (to User Mode)->trap_return_asm