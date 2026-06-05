OUTPUT_ARCH( "riscv" )
ENTRY( _entry )

SECTIONS
{
  /* QEMU 模拟器规定的物理内存起点 */
  . = 0x80000000;

  /* 强行把 entry.S 里的入口代码放在最前面 */
  .text : {
    *(.text.entry)
    *(.text .text.*)
  }

  /* 剩下的常数和变量，跟着往后排就行 */
  .rodata : { *(.rodata .rodata.*) }
  .data : { *(.data .data.*) }
  .bss : { *(.bss .bss.*) }
}
