.section .text 
.align 4 
.global switch_to
# 
switch_to:
 // 第一次存入 trap_handler里面的switch_to调用的下一句给dummy node  ra 是fork_ret(里面去到s1(task_func)执行) 
 // 第二次 进入的ra是和上面一致  ra加载fork_ret (里面去到 新的task_func执行)
 // 第三次                      ra加载（）
  sw ra,0(a0)
  sw sp,4(a0)
  sw s0,8(a0)
  sw s1,12(a0)
  sw s2,16(a0)
  sw s3,20(a0)
  sw s4,24(a0)
  sw s5,28(a0)
  sw s6,32(a0)
  sw s7,36(a0)
  sw s8,40(a0)
  sw s9,44(a0)
  sw s10,48(a0)
  sw s11,52(a0)
 
  lw ra,0(a1)
  lw sp,4(a1)
  lw s0,8(a1)
  lw s1,12(a1)
  lw s2,16(a1)
  lw s3,20(a1)
  lw s4,24(a1)
  lw s5,28(a1)
  lw s6,32(a1)
  lw s7,36(a1)
  lw s8,40(a1)
  lw s9,44(a1)
  lw s10,48(a1)
  lw s11,52(a1)


  ret 
