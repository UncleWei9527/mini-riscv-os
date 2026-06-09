# 极简清爽版 Makefile (注意命令前必须是真实的 Tab 键)

CC = riscv64-unknown-elf-gcc
CFLAGS = -march=rv32ima_zicsr -mabi=ilp32 -mcmodel=medany -ffreestanding -nostdlib -O0 -g
QEMU = qemu-system-riscv32
QFLAGS = -machine virt -bios none -kernel os.elf -nographic
INC = src/def.h 
SRC = src/stdio.c src/malloc.c  src/entry.S src/start.c src/main.c  src/trap.S  src/switch.S 
LD  = os.ld 
# 1. 核心编译规则
os.elf: $(SRC)  $(INC) $(LD)
	$(CC) $(CFLAGS) -T  $(LD) $(SRC) -o os.elf

# 2. 清理
clean:
	rm -f os.elf

# 3. 一键运行
run: os.elf
	$(QEMU) $(QFLAGS)

# 4. 启动调试服务端 (在这里冻结，等待连接)
# 注意：我把原来的 & 删掉了，因为你现在可以用 LazyVim 开左右两个终端了！
debug: os.elf
	$(QEMU) $(QFLAGS) -s -S

# 5. 启动 GDB 客户端 (在 LazyVim 的另一个分屏终端里运行)
gdb:
	gdb-multiarch os.elf -ex "target remote localhost:1234" -ex "b main" -ex "layout src"
# 危险而暴力的单终端一键调试
oneline-debug: os.elf
	
	@echo "启动 QEMU 放入后台..."
	$(QEMU) $(QFLAGS) -S -s &
	@sleep 1
	@echo "启动 GDB..."
	gdb-multiarch os.elf -ex "target remote localhost:1234" -ex "b main" -ex "layout src" 
