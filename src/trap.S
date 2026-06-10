.section .text
.align 4
.global trap_vector
trap_vector:
  # 【神来之笔】：原子交换 SP 和 SSCRATCH
  # 交换前：sp 是危险的用户栈，sscratch 保存着当前任务 tf 的地址
  # 交换后：sp 变成了 tf 的地址！sscratch 暂存了用户的 sp！
  csrrw sp, sscratch, sp

  # 1. 此时 sp 是 tf 结构体，直接疯狂保存用户现场
  sw x1, 8(sp)
  sw t0, 24(sp)#提前save t0
  # 将暂存在 sscratch 里的用户 SP 拿出来，存入 tf 的 x2_sp 字段 (偏移12)
  csrr t0, sscratch
  sw t0, 12(sp)

  sw x3, 16(sp)
  sw x4, 20(sp)
  sw x6, 28(sp)
  sw x7, 32(sp)
  sw x8, 36(sp)
  sw x9, 40(sp)
  sw x10, 44(sp)
  sw x11, 48(sp)
  sw x12, 52(sp)
  sw x13, 56(sp)
  sw x14, 60(sp)
  sw x15, 64(sp)
  sw x16, 68(sp)
  sw x17, 72(sp)
  sw x18, 76(sp)
  sw x19, 80(sp)
  sw x20, 84(sp)
  sw x21, 88(sp)
  sw x22, 92(sp)
  sw x23, 96(sp)
  sw x24, 100(sp)
  sw x25, 104(sp)
  sw x26, 108(sp)
  sw x27, 112(sp)
  sw x28, 116(sp)
  sw x29, 120(sp)
  sw x30, 124(sp)
  sw x31, 128(sp)

  # 2. 保存用户被打断时的 PC到epc
  csrr t0, sepc
  sw t0, 4(sp)

  # 3. 气闸舱关闭，加载真正的内核栈！
  # tf 的第 0 个字节 (kernel_sp) 存的就是内核栈地址
  lw sp, 0(sp)
  
  # 直接跳入 C 语言内核 (注意：这里不需要传参了)
  call trap_handler
# 专属的返回函数 (类似 xv6 的 userret)
.align 4
.global trap_return_asm
trap_return_asm:
  # C 语言调用时会把 tf 的地址通过 a0 传进来
  mv sp, a0

  # 1. 恢复 sepc
  lw t0, 4(sp)
  csrw sepc, t0

  # 2. 保存an当期trap_frame到sscratch里面
  csrw sscratch, sp

  # 3. 恢复通用寄存器
  lw x1, 8(sp)
  lw x3, 16(sp)
  lw x4, 20(sp)
  lw x5, 24(sp)
  lw x6, 28(sp)
  lw x7, 32(sp)
  lw x8, 36(sp)
  lw x9, 40(sp)
  lw x10, 44(sp)
  lw x11, 48(sp)
  lw x12, 52(sp)
  lw x13, 56(sp)
  lw x14, 60(sp)
  lw x15, 64(sp)
  lw x16, 68(sp)
  lw x17, 72(sp)
  lw x18, 76(sp)
  lw x19, 80(sp)
  lw x20, 84(sp)
  lw x21, 88(sp)
  lw x22, 92(sp)
  lw x23, 96(sp)
  lw x24, 100(sp)
  lw x25, 104(sp)
  lw x26, 108(sp)
  lw x27, 112(sp)
  lw x28, 116(sp)
  lw x29, 120(sp)
  lw x30, 124(sp)
  lw x31, 128(sp)

  # 4. 最后一步，恢复用户的真实 SP
  lw sp, 12(sp)

  # 降维飞回用户态
  sret



  .align 4 
  .global timervec 
  timervec:
    #只动用了 a0 ,a1,a2,a3  只保存这几个 
    csrrw a0,mscratch,a0 

    sw a1,0(a0)
    sw a2,4(a0)
    sw a3,8(a0)

  #重置闹钟 
    lw a1,12(a0)#timcmp地址
    lw a2,16(a0)#闹钟间隔时间

    lw a3,0(a1) #闹钟低位 
    add a3,a3,a2 
    sw a3,0(a1)

    sltu a2,a3,a2#判断是否进位
    lw a3,4(a1)#进高位
    add a3,a3,a2 #a2表示是否进位 
    sw a3,4(a1)#更新回timecmp 低位
    # 模拟软中断给S态
    li a1,2 
    csrw sip,a1 

    lw a3,8(a0)
    lw a2,4(a0)
    lw a1,0(a0)
    csrrw a0,mscratch ,a0 
    mret 
    
