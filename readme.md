# 内存段分类

1. .text 只读代码段
2. .rodata 只读数据段
3. .data 初始化全局变量段
4. .bss 未初始全局变量段
# RISC-V (RV32) 操作系统底层核心特权与寄存器全景手册

> **写在前面的话**：开发操作系统就是与 CPU 硬件制定契约。本手册提炼了从裸机点亮到构建多任务系统所必需的最核心 RISC-V 底层机制。所有寄存器均基于 32 位 (RV32) 架构。

---

## 零、 核心基石：三大特权级 (Privilege Levels)

在执行指令前，必须明确当前代码处于哪个“阶层”：

* **M-Mode (Machine, 机器模式 / 编码 `11`)**：最高权限，拥有上帝视角。负责刚通电时的硬件初始化，随后将权力下放。
* **S-Mode (Supervisor, 特权模式 / 编码 `01`)**：**操作系统的核心阵地**。大管家，负责管理内存隔离（页表）、调度进程、接管中断。
* **U-Mode (User, 用户模式 / 编码 `00`)**：最低权限。普通的 App 运行地，被严格限制，无法直接操作硬件。

---

## 一、 核心特权指令：操作硬件的“咒语”

这些指令只能在特权模式下（或特定的汇编环境）使用，是操作系统掌控 CPU 的动作核心。

| 汇编指令 | 英文全称 | 核心用途与机制 |
| :--- | :--- | :--- |
| **`csrr`** | CSR Read | **读取控制面板**：将某个 CSR 寄存器的值读入通用寄存器（如 `csrr t0, mstatus`）。 |
| **`csrw`** | CSR Write | **覆写控制面板**：将通用寄存器的值写入 CSR 寄存器，完全覆盖旧值。 |
| **`csrs`** | CSR Set | **按位置 1 (开开关)**：将 CSR 中特定的几位设为 `1`，其余位保持不变。 |
| **`csrc`** | CSR Clear | **按位清 0 (关开关)**：将 CSR 中特定的几位清零，其余位保持不变。 |
| **`mret`** | Machine Return | **M模式返回/降级**：硬件读取 `mstatus.MPP` 决定降级目标模式，并跳转至 `mepc` 记录的地址。 |
| **`sret`** | Supervisor Return | **S模式返回/降级**：硬件读取 `sstatus.SPP` 决定降级目标模式，并跳转至 `sepc` 记录的地址（常用于启动用户进程）。 |
| **`ecall`** | Environment Call | **主动呼救铃 (系统调用)**：低权限代码主动触发异常，强制将 CPU 控制权交还给上级模式（如 U 呼叫 S）。 |

---

## 二、 核心特权寄存器 (CSR)：CPU 的状态面板

> ⚠️ **注意**：在 RV32 架构下，所有的寄存器位宽均为 32 Bit。

### 1. 状态总控台：`mstatus` / `sstatus`
最复杂的寄存器，记录全局开关和特权级跃迁历史。`sstatus` 是 `mstatus` 的受限子集。

| 位 (Bit) | 字段名 | 控制对象 | 核心机制与用途 |
| :--- | :--- | :--- | :--- |
| **Bit 3** | **MIE** | M模式 全局中断使能 | `1` 为开，`0` 为关。Boot 阶段通常设为 `0` 以防止干扰。 |
| **Bit 7** | **MPIE** | M模式 昔日中断记录 | Trap 发生时，硬件自动将案发前的 `MIE` 状态备份至此；`mret` 时恢复。 |
| **Bits 11:12**| **MPP** | M模式 昔日特权级 | 记录 Trap 发生前身处的模式 (`00`=U, `01`=S, `11`=M)。**Boot 降级时手动改写为 `01`。** |
| **Bit 1** | **SIE** | S模式 全局中断使能 | 操作系统层的“总电闸”。 |
| **Bit 8** | **SPP** | S模式 昔日特权级 | S模式下仅占用 1 位（`0`代表来自 U模式，`1`代表来自 S模式）。 |

### 2. 异常与中断处理 (Trap Handlers)
当“意外”或“呼叫”发生时，这组寄存器负责记录案发现场并指引救援。

| 寄存器族 | 英文缩写 | 核心用途与字段解析 |
| :--- | :--- | :--- |
| **`mepc` / `sepc`** | Exception Program Counter | **时光传送门（断点地址）**：存放被打断指令的 32 位内存地址。`mret`/`sret` 将跳回此地址。 |
| **`mcause` / `scause`**| Cause | **案发原因报告**：<br>• **Bit 31 (最高位)**：`1` 代表中断(Interrupt)，`0` 代表异常(Exception)。<br>• **Bits 0:30**：具体错误码。如代码 `8` 是来自 U-Mode 的 `ecall`，代码 `5` 是时钟中断。 |
| **`mtvec` / `stvec`** | Trap Vector Base Address| **接线员的工位（入口地址）**：<br>• **Bits 2:31**：处理异常的 C/汇编函数基地址。<br>• **Bits 0:1**：跳转模式。`00` 为 Direct 模式（统一跳转交由软件分发），`01` 为 Vectored 向量模式。 |

### 3. 具体中断开关：`mie` / `sie`
部门级分电闸。只有当全局使能开启，且此处对应的位也开启时，中断才能被 CPU 接收。

| 位 (Bit) | 字段名 (S模式) | 中断类型 | 实战意义 |
| :--- | :--- | :--- | :--- |
| **Bit 1** | **SSIE** | 软件中断 (Software) | 多核 CPU 间通信（IPI）。 |
| **Bit 5** | **STIE** | 时钟中断 (Timer) | **操作系统的“心脏起搏器”**，实现多任务时间片轮转。 |
| **Bit 9** | **SEIE** | 外部中断 (External) | 处理外部硬件输入（键盘、鼠标、磁盘、网卡等）。 |
*(注：对应的 M 模式开关为 MSIE (Bit 3)、MTIE (Bit 7)、MEIE (Bit 11))*

### 4. 物理内存保护：PMP (仅 M-Mode 可见)
防止低权限模式（S/U）越权访问物理内存。若不配置，下级模式无法访问任何内存。

| 寄存器 | 全称 | 字段解析与实战配置 |
| :--- | :--- | :--- |
| **`pmpcfg0`**| PMP Configuration | **权限配置表**：管理第 0 号内存区域（最低 8 Bits）。<br>• `Bit 0 (R)`: 读权限<br>• `Bit 1 (W)`: 写权限<br>• `Bit 2 (X)`: 执行权限<br>• `Bits 3:4 (A)`: 地址匹配模式。**设为 `01` 代表 TOR 模式（从 0 地址到 pmpaddr 指定的界限）。** |
| **`pmpaddr0`**| PMP Address | **领地界碑**：划定授权范围的物理地址。<br>• **实战**：写入 `0xffffffff`（32位最大值），配合 `pmpcfg0` 的 R/W/X 和 TOR 模式，实现对下级模式的**内存全盘放权**。 |
# RISC-V 陷阱委托机制：medeleg 与 mideleg 寄存器详解

> **核心哲学**：在 RISC-V 系统中，M-Mode（机器模式）默认是所有异常和中断的“终极兜底方”。但为了提升系统性能并让 S-Mode（操作系统）实现真正的自治，M-Mode 必须通过**委托（Delegation）**机制，将特定类型的异常和中断处理权下放给 S-Mode。

---

## 一、 为什么需要 Delegation（委托）？

在没有委托机制的默认状态下，硬件的路由规则极其霸道：**无论当前 CPU 处于什么特权级（U 或 S），只要发生任何异常（Exception）或中断（Interrupt），硬件都会立刻将特权级强行提升到 M-Mode。**

如果按照这个默认规则，操作系统（S-Mode）处理用户程序（U-Mode）系统调用的流程将变得极其低效：
1. U-Mode 程序执行 `ecall`。
2. CPU 强行跳转到 M-Mode 的处理函数。
3. M-Mode 软件发现这是一个 U-Mode 触发的系统调用。
4. M-Mode 软件手动修改寄存器，将 CPU 重新跳转回 S-Mode 的内核去处理。

这种“凡事都要董事长（M-Mode）过问，再转交给总经理（S-Mode）”的设计会带来巨大的性能损耗。因此，RISC-V 引入了 `medeleg` 和 `mideleg` 这两个 M-Mode 专属控制寄存器。

**核心原理：** 寄存器中的第 `N` 位（Bit），就代表着错误码（Cause Code）为 `N` 的异常或中断。
* 当 Bit `N` 为 `0` 时：发生该异常/中断时，CPU 默认跳入 M-Mode。
* 当 Bit `N` 为 `1` 时：发生该异常/中断时，如果当前处于 S-Mode 或 U-Mode，CPU 会直接跳入 S-Mode（通过 `stvec`），再也不去打扰 M-Mode。

---

## 二、 `medeleg` (Machine Exception Delegation) 异常委托

`medeleg` 专门负责“同步异常”的权力下放。我们在内核代码中写入的 `0xffff`（即把低 16 位全部置为 1），意味着把绝大多数标准异常都交给了 S-Mode 操作系统去处理。

下表列出了 `medeleg` 寄存器中，最核心的几个异常委托位（Bit位与 `mcause` 中的异常代码完全对应）：

| 位 (Bit) | 异常原因 (Exception Cause) | 大白话解释与实战意义 |
| :--- | :--- | :--- |
| **Bit 0** | 指令地址未对齐 | 试图从不能被 4（或 2）整除的地址读取指令。 |
| **Bit 1** | 指令访问错误 | 试图执行没有执行权限的内存（比如撞到了 PMP 门禁）。 |
| **Bit 2** | 非法指令 | CPU 读到了根本不存在的“假指令”，或者是 U 模式试图执行特权指令。 |
| **Bit 3** | 断点 (Breakpoint) | GDB 调试器打断点触发的异常（`ebreak` 指令）。 |
| **Bit 8** | U-Mode 环境呼叫 | **极其重要！** U-Mode 用户程序主动执行 `ecall` 发起系统调用。 |
| **Bit 9** | S-Mode 环境呼叫 | S-Mode 自己主动执行 `ecall` 触发异常。 |
| **Bit 12** | 指令页错误 (Page Fault) | 虚拟内存机制：试图执行当前不在物理内存中的代码。 |
| **Bit 13** | 读取页错误 (Load Page Fault) | 虚拟内存机制：试图读取不在内存中的数据，触发页面置换（Swap）。 |
| **Bit 15** | 写入页错误 (Store Page Fault)| 虚拟内存机制：经典的 Copy-On-Write (写时复制) 触发机制。 |

> **⚠️ 权限天花板法则**：M-Mode 的环境呼叫（`ecall` from M-Mode，即 Bit 11）是**绝对不允许**被下放到 S-Mode 的。硬件强制规定高级别的异常不能交给低级别处理。

---

## 三、 `mideleg` (Machine Interrupt Delegation) 中断委托

`mideleg` 专门负责“异步中断”的权力下放。与异常不同，中断是由外部定时器或硬件设备触发的。

它的位映射规则稍有不同，它与 `mie` / `mip` 寄存器中的中断开关位保持一致：

| 位 (Bit) | 中断类型 (Interrupt Cause) | 大白话解释与实战意义 |
| :--- | :--- | :--- |
| **Bit 1** | SSIP (S-Mode 软件中断) | 主要用于多核 CPU 架构下，一个核心向另一个核心发送消息（IPI）。 |
| **Bit 5** | STIP (S-Mode 定时器中断) | **操作系统的“心跳”！** 下放此权限，S-Mode 才能自主处理时间片轮转和多任务进程调度。 |
| **Bit 9** | SEIP (S-Mode 外部中断) | 下放此权限，S-Mode 才能直接接收并处理键盘敲击、网卡数据、硬盘读写完成等外部硬件信号。 |

*(注：Bit 3, 7, 11 对应的是 M-Mode 的软件、定时、外部中断，这些位在 `mideleg` 中是只读的 `0`，硬件不允许把 M-Mode 的专属中断下放给 S-Mode。)*

---

## 四、 硬件的“智能路由”判断逻辑 (Trap Routing)

当一个 Trap（错误码为 `N`）发生时，CPU 内部的晶体管会进行如下飞速的逻辑判断：

1. **检查当前状态**：Trap 发生时，我当前是处在 M-Mode、S-Mode 还是 U-Mode？
2. **如果在 M-Mode**：无视一切委托规则，直接跳向 `mtvec`（M-Mode 接线员）。
3. **如果在 S-Mode 或 U-Mode**：
   * 硬件立刻去查阅委托登记表。
   * 如果这是一个异常，查看 `medeleg` 的第 `N` 位；如果这是一个中断，查看 `mideleg` 的第 `N` 位。
   * **如果该位为 `0`**：上报董事长，立刻升级到 M-Mode，跳向 `mtvec`。
   * **如果该位为 `1`**：启动直达通道，进入 S-Mode，跳向 `stvec` 寻找操作系统的接线员！

## 五、 总结与实战启示

在构建我们自己的 RISC-V 操作系统 Bootloader（如 `start.c`）时，执行：
```c
w_medeleg(0xffff);
w_mideleg(0xffff);
```
 
 # RISC-V 陷阱 (Trap) 机制与 scause 寄存器详解

> **核心哲学**：在 RISC-V 架构中，所有打断 CPU 正常执行流的突发事件统称为 **Trap（陷阱）**。当 S-Mode 操作系统接管 Trap 时，硬件会将具体的“案发原因”自动记录在 `scause` (Supervisor Cause) 寄存器中。内核的 `trap_handler` 就是通过解析这个寄存器来决定下一步行动的。

---

## 一、 中断 (Interrupt) 与 异常 (Exception) 的本质区别

Trap 被严格划分为两大家族。区分它们的唯一标准，就是看 `scause` 寄存器的**最高位（Bit 31，假设为 32 位系统）**：

* **最高位 = 1（中断 Interrupt）**：
    * **特征**：异步发生，与 CPU 当前正在执行的指令无关。
    * **来源**：外部硬件（定时器、键盘、网卡等）。
* **最高位 = 0（异常 Exception）**：
    * **特征**：同步发生，一定是 CPU 刚刚试图执行的那条指令“惹的祸”或“主动发起的请求”。
    * **来源**：非法操作（越界、缺页）、系统调用（ecall）。

---

## 二、 scause 报警代码速查表

通过将 `scause` 的最高位清零（`scause & 0x7fffffff`），我们可以提取出真实的**异常/中断代码 (Cause Code)**。

### 1. 中断家族 (Interrupts, Bit 31 = 1)

| Cause Code | 英文全称 | 中文翻译 | 操作系统实战意义 |
| :--- | :--- | :--- | :--- |
| **1** | Supervisor Software Interrupt | **S 模式软件中断** | 主要用于多核 CPU 架构。例如 CPU-0 给 CPU-1 发送跨核消息 (IPI) 以唤醒对方。 |
| **5** | Supervisor Timer Interrupt | **S 模式定时器中断** | **多任务操作系统的心脏！** 依靠此中断，内核可以强行剥夺当前进程的 CPU 使用权，实现时间片轮转调度。 |
| **9** | Supervisor External Interrupt | **S 模式外部中断** | **设备驱动的命脉！** 键盘敲击、鼠标移动、磁盘读写完成等外部硬件信号，均通过此通道通知内核。 |

### 2. 异常家族 (Exceptions, Bit 31 = 0)

异常又可以细分为三大类：致命错误、主动请求与虚拟内存魔法。

#### A. 内存与指令错误类（通常会导致程序崩溃退出）
| Cause Code | 英文全称 | 中文翻译与大白话解释 |
| :--- | :--- | :--- |
| **0** | Instruction Address Misaligned | **指令地址未对齐**：试图从非 4 字节（或 2 字节）对齐的地址读取指令。 |
| **1** | Instruction Access Fault | **指令访问错误**：试图执行没有权限的内存区域代码（如触发 PMP 物理门禁拦截）。 |
| **2** | Illegal Instruction | **非法指令**：CPU 遇到无法解码的机器码，或低权限模式企图执行特权指令。 |
| **4** | Load Address Misaligned | **读取地址未对齐**：试图从不对齐的地址加载数据。 |
| **5** | Load Access Fault | **读取访问错误**：读取越界或缺乏读取权限。 |
| **6** | Store Address Misaligned | **写入地址未对齐**：写入时目标地址不对齐。 |
| **7** | Store Access Fault | **写入访问错误**：向无写权限的内存写入数据（例如企图修改只读代码段或 ROM）。 |

#### B. 系统调用类（程序的合法求助通道）
| Cause Code | 英文全称 | 中文翻译与大白话解释 |
| :--- | :--- | :--- |
| **8** | Environment Call from U-mode | **来自 U 模式的系统调用**：用户程序主动执行 `ecall` 指令。这是操作系统为普通 App 提供服务（如 `printf`、`open`、`read`）的唯一合法入口。 |
| **9** | Environment Call from S-mode | **来自 S 模式的系统调用**：S-Mode 内核自身执行 `ecall` 触发的异常。 |

#### C. 虚拟内存类（现代 OS 内存管理的核心）
| Cause Code | 英文全称 | 中文翻译与实战意义 |
| :--- | :--- | :--- |
| **12** | Instruction Page Fault | **指令页错误**：试图执行的代码目前不在物理内存中。 |
| **13** | Load Page Fault | **读取页错误**：试图读取的数据不在物理内存中。 |
| **15** | Store Page Fault | **写入页错误**：试图写入的地址不在内存中，或触发了写保护（这是实现 Copy-On-Write 写时复制机制的核心）。 |
> **💡 黑客提示**：页错误 (Page Fault) 并不意味着程序一定会崩溃。优秀的操作系统内核会拦截这个异常，悄悄从硬盘 Swap 区把数据搬回物理内存，更新页表，然后让 CPU 重新执行触发异常的指令，做到对用户程序完全透明。

---

## 三、 C 语言解析模板 (trap_handler)

配合底层的 `trap.S` 汇编上下文保护，S-Mode 下的 C 语言总接线员可以通过以下标准逻辑进行事件分发：

```c
// 从硬件读取案发现场信息
static inline unsigned int r_scause() {
    unsigned int x;
    asm volatile("csrr %0, scause" : "=r"(x));
    return x;
}

void trap_handler() {
    unsigned int cause = r_scause();
    
    // 解析最高位：1 为中断，0 为异常
    int is_interrupt = (cause & 0x80000000) != 0;
    // 屏蔽最高位，提取真实的错误码
    unsigned int cause_code = cause & 0x7fffffff;

    if (is_interrupt) {
        // --- 路由到中断处理逻辑 ---
        if (cause_code == 5) {
            // 处理定时器中断 (进程切换)
        } else if (cause_code == 9) {
            // 处理外部中断 (硬件驱动)
        }
    } else {
        // --- 路由到异常处理逻辑 ---
        if (cause_code == 8 || cause_code == 9) {
            // 处理系统调用 (System Call)
        } else {
            // 处理致命异常或页错误 (Panic or Page Fault)
        }
    }
}
