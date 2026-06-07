# RISC-V 核心特权寄存器 (CSR) 官方级位域全景手册 (RV32 专版)

## 1. 状态寄存器 (Status Registers)
控制 CPU 全局执行状态、特权级、中断全局开关及内存访问权限。在 RV32 环境下，寄存器总宽度为 32 位。

### 1.1 `mstatus` (Machine Status Register)
*M模式下可读写。RV32 环境下的严格 32 位布局。*

| 位域 (Bits) | 名称 (Name) | 功能详述 (Official Description) |
| :--- | :--- | :--- |
| **0** | **UIE** | **U模式中断开关 (User Interrupt Enable)**。0=关闭，1=开启。（N扩展专用，通常不启用） |
| **1** | **SIE** | **S模式中断开关 (Supervisor Interrupt Enable)**。0=全局关闭S态中断，1=全局开启S态中断。 |
| **2** | *WPRI* | 保留位 (Reserved)。 |
| **3** | **MIE** | **M模式中断开关 (Machine Interrupt Enable)**。0=全局关闭M态中断，1=全局开启M态中断。 |
| **4** | **UPIE** | **U模式前中断开关 (User Previous IE)**。进入U态异常前自动保存的 UIE 值。 |
| **5** | **SPIE** | **S模式前中断开关 (Supervisor Previous IE)**。进入S态异常前自动保存的 SIE 值。`sret` 时将其恢复给 SIE。 |
| **6** | **UBE** | **U模式字节序 (User Big-Endian)**。0=小端，1=大端。 |
| **7** | **MPIE** | **M模式前中断开关 (Machine Previous IE)**。进入M态异常前自动保存的 MIE 值。`mret` 时将其恢复给 MIE。 |
| **8** | **SPP** | **S模式前特权级 (Supervisor Previous Privilege)**。记录进入S态异常前的特权级：0=User，1=Supervisor。`sret` 时降级到此状态。 |
| **9~10** | **VS** | **矢量扩展状态 (Vector Status)**。管理 Vector 扩展单元的状态 (00=Off, 01=Initial, 10=Clean, 11=Dirty)。 |
| **11~12** | **MPP** | **M模式前特权级 (Machine Previous Privilege)**。记录进入M态异常前的特权级：00=U态，01=S态，11=M态。`mret` 时降级到此状态。 |
| **13~14** | **FS** | **浮点单元状态 (Floating-Point Status)**。管理 FPU 状态 (00=Off, 01=Initial, 10=Clean, 11=Dirty)。 |
| **15~16** | **XS** | **用户扩展状态 (User Extension Status)**。管理额外用户态扩展状态。 |
| **17** | **MPRV** | **修改特权级访问 (Modify Privilege)**。设为 1 时，Load/Store 将使用 `MPP` 中的特权级来检查内存读写权限。 |
| **18** | **SUM** | **允许S态访问U态内存 (Permit Supervisor User Memory access)**。设为 0 时，S态程序如果访问U态内存页会直接触发异常；1 为允许。 |
| **19** | **MXR** | **可执行即为可读 (Make Executable Readable)**。设为 1 时，允许读取页表中标记为“仅执行 (X=1, R=0)”的内存页。 |
| **20** | **TVM** | **捕获虚拟内存管理 (Trap Virtual Memory)**。设为 1 时，S态读写 `satp` 或执行 `sfence.vma` 会触发异常（交由 M态处理）。 |
| **21** | **TW** | **超时等待 (Timeout Wait)**。设为 1 时，S态执行 `WFI` 如果超时会被 M态拦截触发异常。 |
| **22** | **TSR** | **捕获 SRET (Trap SRET)**。设为 1 时，禁止 S态 执行 `sret`，一旦执行直接触发异常。 |
| **23~30** | *WPRI* | 保留位 (Reserved)。 |
| **31** | **SD** | **状态脏位 (State Dirty)**。(RV32 的最高位) 只读。若 `FS`、`VS` 或 `XS` 中有任意一个状态为 11 (Dirty)，则该位置 1。 |

### 1.2 `sstatus` (Supervisor Status Register)
*物理上，`sstatus` 是 `mstatus` 的受限影子。M模式 专有权限的位域（如 MIE, MPIE, MPP 等）在读取 `sstatus` 时强制为 0，且不可修改。*

---

## 2. 中断控制寄存器 (Interrupt Registers)
控制各种异步中断开关 (`ie`) 和中断记录是否来了 (`ip`)。
*注：32位下的布局与位号依然完全对齐。*

| 位号 (Bits) | `mie` / `sie` (中断开关) | `mip` / `sip` (中断需要关闭) | 功能描述 (Interrupt Type) |
| :--- | :--- | :--- | :--- |
| **0** | USIE | USIP | **U态软件中断 (User Software Interrupt)**。 |
| **1** | **SSIE** | **SSIP** | **S态软件中断 (Supervisor Software Interrupt)**。S态大管家门铃，S态需手动清0。 |
| **3** | **MSIE** | **MSIP** | **M态软件中断 (Machine Software Interrupt)**。多核间发送的中断 (IPI)。 |
| **4** | UTIE | UTIP | **U态定时器中断 (User Timer Interrupt)**。 |
| **5** | **STIE** | **STIP** | **S态定时器中断 (Supervisor Timer Interrupt)**。 |
| **7** | **MTIE** | **MTIP** | **M态定时器中断 (Machine Timer Interrupt)**。由 `mtime >= mtimecmp` 触发，纯只读。 |
| **8** | UEIE | UEIP | **U态外部中断 (User External Interrupt)**。 |
| **9** | **SEIE** | **SEIP** | **S态外部中断 (Supervisor External Interrupt)**。由 PLIC 路由给 S 态的外设中断（键盘、串口等）。 |
| **11** | **MEIE** | **MEIP** | **M态外部中断 (Machine External Interrupt)**。由 PLIC 路由给 M 态的外部中断。 |

---

## 3. 陷阱与寻址寄存器 (Trap & Control Flow Registers)

### 3.1 `mtvec` / `stvec` (Trap-Vector Base-Address)
*控制发生异常/中断时，硬件自动跳转的入口地址。*

| 位域 (Bits) | 名称 | 功能描述 |
| :--- | :--- | :--- |
| **2 ~ 31** | **BASE** | **陷阱向量基地址 (Base Address)**。必须按 4 字节对齐。 |
| **0 ~ 1** | **MODE** | **跳转模式 (Vector Mode)**。<br>• `00`：Direct (直接模式)。所有情况跳向 `BASE`。<br>• `01`：Vectored (向量模式)。异常跳向 `BASE`；中断跳向 `BASE + (4 × cause)`。 |

### 3.2 `mepc` / `sepc` (Exception Program Counter)
* 记录被打断前指令的物理/虚拟地址（32位宽度）。
* 全局单例，多任务切换时必须保存至进程的 `trapframe` 中。

### 3.3 `mscratch` / `sscratch` (Scratch Register)
* **硬件毫无约定的 32 位纯数据寄存器。**
* 常利用 `csrrw` (原子读写) 交换上下文指针，获取内核栈地址。

---

## 4. 异常原因寄存器 (`mcause` / `scause`)
*记录打断 CPU 执行的原因。*

| 位域 (Bits) | 名称 | 功能描述 |
| :--- | :--- | :--- |
| **31** *(最高位)* | **Interrupt** | **中断标志位**。1 代表异步中断 (外部/定时/软件)；0 代表同步异常 (指令错误/系统调用)。 |
| **0 ~ 30** | **Exception Code** | **异常/中断错误码 (Exception Code)**。详情见下表。 |

### 核心 Exception Code 错误码速查表
| 中断标志 (Bit 31) | 错误码 (Bits 0-30) | 详细原因 (Description) | 分类 |
| :---: | :---: | :--- | :--- |
| 1 | 1 | Supervisor software interrupt (S态软件呼叫) | 异步中断 |
| 1 | 3 | Machine software interrupt (M态核间呼叫) | 异步中断 |
| 1 | 5 | Supervisor timer interrupt (S态定时器) | 异步中断 |
| 1 | 7 | Machine timer interrupt (M态物理闹钟) | 异步中断 |
| 1 | 9 | Supervisor external interrupt (S态外部设备) | 异步中断 |
| 1 | 11 | Machine external interrupt (M态外部设备) | 异步中断 |
| 0 | 0 | Instruction address misaligned (指令地址非对齐) | 同步异常 |
| 0 | 1 | Instruction access fault (指令访问总线错误) | 同步异常 |
| 0 | 2 | Illegal instruction (非法指令/未定义指令) | 同步异常 |
| 0 | 3 | Breakpoint (断点 EBREAK 指令) | 同步异常 |
| 0 | 4 | Load address misaligned (读取内存地址非对齐) | 同步异常 |
| 0 | 5 | Load access fault (读取内存总线错误) | 同步异常 |
| 0 | 6 | Store/AMO address misaligned (写/原子操作地址非对齐) | 同步异常 |
| 0 | 7 | Store/AMO access fault (写/原子操作总线错误) | 同步异常 |
| 0 | 8 | Environment call from U-mode (用户态执行 ECALL) | **系统调用** |
| 0 | 9 | Environment call from S-mode (S态执行 ECALL) | **系统调用** |
| 0 | 11 | Environment call from M-mode (M态执行 ECALL) | **系统调用** |
| 0 | 12 | Instruction page fault (取指令触发页异常) | **虚拟内存页异常** |
| 0 | 13 | Load page fault (读内存触发页异常) | **虚拟内存页异常** |
| 0 | 15 | Store/AMO page fault (写内存触发页异常) | **虚拟内存页异常** |

---

## 5. 权限下放/委托寄存器 (Delegation Registers)

### 5.1 `mideleg` (Machine Interrupt Delegation)
* 位数与 `mip` 严格对应（32位）。
* 若对应位为 1，则该异步中断将越过 M态，直接陷入 S态 (`stvec`) 处理。

### 5.2 `medeleg` (Machine Exception Delegation)
* 位数与 `mcause` 中的 Exception Code 严格对应。
* 置 1 对应的同步异常（如 `ecall from U-mode`）将直接触发 S态 异常陷阱，这是实现操作系统系统调用的硬件基石。
## 6. 物理内存保护寄存器 (Physical Memory Protection - PMP)
*M模式专有寄存器。用于配置 S态 和 U态 对物理内存的访问权限。RISC-V 默认“拒绝一切访问”，M 态必须显式开通权限，否则 S/U 态寸步难行。*

### 6.1 `pmpcfgX` (PMP Configuration Registers)
*在 32 位系统 (RV32) 中，每个 `pmpcfgX` 寄存器长 32 位，内部打包了 **4 个 8位** 的子配置项（例如 `pmpcfg0` 包含了 `pmp0cfg` 到 `pmp3cfg`）。*

**单个 8位 PMP 配置项 (`pmpXcfg`) 的位域解析：**

| 位号 (Bits) | 名称 | 功能描述 |
| :--- | :--- | :--- |
| **0** | **R (Read)** | **读权限**。1 = 允许 S/U 态读取该内存区域。 |
| **1** | **W (Write)** | **写权限**。1 = 允许 S/U 态写入该内存区域。 |
| **2** | **X (Execute)** | **执行权限**。1 = 允许 S/U 态在该内存区域执行代码。 |
| **3~4** | **A (Address Mode)** | **地址匹配模式**。<br>• `00` (**OFF**)：关闭此条 PMP 规则。<br>• `01` (**TOR**)：Top of Range 模式。匹配前一个 PMP 地址到当前地址的范围。<br>• `10` (**NA4**)：自然对齐的 4 字节区域。<br>• `11` (**NAPOT**)：自然对齐的 2 的幂次方区域（>= 8 字节）。 |
| **5~6** | *Reserved* | 保留位。 |
| **7** | **L (Lock)** | **锁定/强制位**。一旦置 1，直到下次 CPU 重启前都无法修改此规则。**更致命的是，一旦置 1，该 PMP 限制也会立刻作用于 M 态自己！** (默认为 0 时，M 态不受限制)。 |

### 6.2 `pmpaddrX` (PMP Address Registers)
*指定物理内存保护的边界地址。*
* **注意**：它存的并不是原始的物理地址，而是**物理地址右移 2 位**的值。
* **联合工作 (以 TOR 模式为例)**：如果 `pmp0cfg` 设为 TOR 模式，那么该条规则将覆盖从物理地址 `0x00000000` 到 `pmpaddr0` 所指向地址的全部区域。

---

### 💡 实战揭秘：为什么开机要写 0xf 和 0xffffffff？
在基础操作系统的 `start.c` 中，我们通常用两行代码暴力破解 PMP 封锁，实现“全盘绿灯”：
1. `w_csr(pmpaddr0, 0xffffffff);`
   *(把地址边界设到极大值，涵盖整个 4GB 乃至更大的物理内存空间)*
2. `w_csr(pmpcfg0, 0xf);`
   *(十进制 15，二进制 `0b00001111`)*
   *(对照上表：R=1, W=1, X=1，并且 A=01 代表 TOR 模式，L=0 不锁定)*
   **最终效果**：告诉 CPU，从 `0` 到 `0xffffffff` 的整块内存，S 态和 U 态都可以随意读、写、执行！