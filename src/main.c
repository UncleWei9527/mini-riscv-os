#define UART0 0x10000000L
#define UARTC (*(volatile unsigned char *)(UART0))
void uart_putc(char c) { UARTC = c; }
void print_string(const char *s) {
  while (*s) {
    uart_putc(*s);
    s++;
  }
}
int main() {
  print_string("Hello RISCV OS!!!");
  while (1) {
  }
  return 0;
}
