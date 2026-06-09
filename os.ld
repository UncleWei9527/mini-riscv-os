OUTPUT_ARCH( "riscv" )
ENTRY( _entry )

SECTIONS
{
  /* 1. 定位器（Location Counter）：从 QEMU 规定的物理内存起点开始建城 */
  . = 0x80000000;

  /* 2. 第一块地：代码段 (存放所有汇编和 C 函数指令) */
  .text : {
    *(.text.entry)  /* 强行把 entry.S 塞在绝对的第一位，保证开机就能执行到 */
    *(.text .text.*)
  }

  /* 3. 按 4 字节对齐，防止硬件崩溃 */
  . = ALIGN(4);

  /* 4. 第二块地：只读数据段 (存放 printf 里的常量字符串等) */
  .rodata : { 
    *(.rodata .rodata.*) 
  }

  . = ALIGN(4);

  /* 5. 第三块地：已初始化数据段 (存放比如 int lock = 1; 这种全局变量) */
  .data : { 
    *(.data .data.*) 
  }

  . = ALIGN(4);
.bss : ALIGN(8) {
        __bss_start = .;
        *(.bss .bss.*)
        *(.sbss .sbss.*)   /* <-- 关键：必须收编 Small BSS 段，保护指针 */
        *(COMMON)          /* <-- 关键：必须收编 COMMON 段，保护 task_pool */
        . = ALIGN(8);
        __bss_end = .;     /* 现在 __bss_end 真正指向了所有内核数据的末尾 */
    }
}
