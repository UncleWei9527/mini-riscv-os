
.section .text 
.align 4 
.global trap_vector
trap_vector:
  addi sp,sp,-128 
  
# 除了sp和x0 以外都不用保存
  sw x1, 4(sp)
  sw x3, 12(sp)
  sw x4, 16(sp)
  sw x5, 20(sp)
  sw x6, 24(sp)
  sw x7, 28(sp)
  sw x8, 32(sp)
  sw x9, 36(sp)
  sw x10, 40(sp)
  sw x11, 44(sp)
  sw x12, 48(sp)
  sw x13, 52(sp)
  sw x14, 56(sp)
  sw x15, 60(sp)
  sw x16, 64(sp)
  sw x17, 68(sp)
  sw x18, 72(sp)
  sw x19, 76(sp)
  sw x20, 80(sp)
  sw x21, 84(sp)
  sw x22, 88(sp)
  sw x23, 92(sp)
  sw x24, 96(sp)
  sw x25, 100(sp)
  sw x26, 104(sp)
  sw x27, 108(sp)
  sw x28, 112(sp)
  sw x29, 116(sp)
  sw x30, 120(sp)
  sw x31, 124(sp)
#保存了中断地址
  csrr t0,sepc 
  sw t0,0(sp)
  call trap_handler

  lw t0,0(sp)
  csrw sepc,t0 
  lw x1, 4(sp)
  lw x3, 12(sp)
  lw x4, 16(sp)
  lw x5, 20(sp)
  lw x6, 24(sp)
  lw x7, 28(sp)
  lw x8, 32(sp)
  lw x9, 36(sp)
  lw x10, 40(sp)
  lw x11, 44(sp)
  lw x12, 48(sp)
  lw x13, 52(sp)
  lw x14, 56(sp)
  lw x15, 60(sp)
  lw x16, 64(sp)
  lw x17, 68(sp)
  lw x18, 72(sp)
  lw x19, 76(sp)
  lw x20, 80(sp)
  lw x21, 84(sp)
  lw x22, 88(sp)
  lw x23, 92(sp)
  lw x24, 96(sp)
  lw x25, 100(sp)
  lw x26, 104(sp)
  lw x27, 108(sp)
  lw x28, 112(sp)
  lw x29, 116(sp)
  lw x30, 120(sp)
  lw x31, 124(sp)

  
  addi sp,sp,128 
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
    
