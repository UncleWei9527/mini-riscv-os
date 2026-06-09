#include <stdarg.h>
void sys_print(const char *s) {
  // 🌟 强行命令 GCC：这两个变量必须死死地绑定在 a7 和 a0 物理寄存器上！
  register unsigned int a7 asm("a7") = 1;
  register unsigned int a0 asm("a0") = (unsigned int)s;

  __asm__ volatile("ecall"
                   :
                   : "r"(a7), "r"(a0)
                   : "memory"); // 警告编译器别乱动内存
}

void *malloc(int size) {
  register unsigned int a7 asm("a7") = 2;
  register unsigned int a0 asm("a0") = size;

  // 🌟 "+r"(a0) 极度重要：它表示 a0 既是 ecall 的入口传参，也是出口返回值！
  // 这样大管家返回的地契指针，就会稳稳地顺着 a0 带回给 C 语言！
  __asm__ volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");

  return (void *)a0;
}

void free(void *ptr) {
  register unsigned int a7 asm("a7") = 3;
  register unsigned int a0 asm("a0") = (unsigned int)ptr;

  __asm__ volatile("ecall" : : "r"(a7), "r"(a0) : "memory");
}

void printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buf[256];
  int pos = 0;
  for (int i = 0; fmt[i] != '\0'; i++) {
    if (fmt[i] == '%') {
      i++;
      if (fmt[i] == 'd') {
        int val = va_arg(args, int);
        if (val == 0) {
          buf[pos++] = '0';

        } else {
          char tmp[16];
          int t_pos = 0;
          if (val < 0) {
            buf[pos++] = '-';
            val = -val;
          }
          while (val > 0) {
            tmp[t_pos++] = '0' + val % 10;
            val /= 10;
          }
          while (t_pos) {
            buf[pos++] = tmp[--t_pos];
          }
        }
      } else if (fmt[i] == 's') {
        char *str = va_arg(args, char *);
        while (*str) {
          buf[pos++] = *str++;
        }
      }
    } else {
      buf[pos++] = fmt[i];
    }
  }
  buf[pos] = '\0';
  sys_print(buf);
  va_end(args);
}
