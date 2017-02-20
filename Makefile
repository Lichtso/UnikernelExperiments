HPP_SRC := $(wildcard include/*.hpp) $(wildcard include/Hardware/*.hpp) $(wildcard include/Memory/*.hpp) $(wildcard include/Net/*.hpp)
ANALYZE = $(LLVM_BIN)llvm-objdump -print-imm-hex -d -s -t -triple
COMPILE = $(LLVM_BIN)clang -O1 -Iinclude -c -mlittle-endian -Wall -Wsign-compare -target
COMPILE_CPP = -fno-exceptions -fno-unwind-tables -fno-stack-protector -fno-rtti -ffreestanding -mno-unaligned-access -std=c++1z
TARGET_32 = armv7a-none-eabi
TARGET_64 = arm64

build/%.o : src32/%.s
	$(COMPILE) $(TARGET_32) -o $@ $<

build/%.o : src32/%.cpp $(HPP_SRC)
	$(COMPILE) $(TARGET_32) $(COMPILE_CPP) -o $@ $<

build/%.o : src64/%.s
	$(COMPILE) $(TARGET_64) -o $@ $<

build/%.o : src64/%.cpp $(HPP_SRC)
	$(COMPILE) $(TARGET_64) $(COMPILE_CPP) -o $@ $<

build/Bootloader.bin: build/Entry32.o build/Entry64.o build/Bootloader.o
	LD=$(LLVM_BIN)lld tools/build_tool/target/release/build_tool 0x10000 0x2000 $@ $^

build/Kernel.bin: build/Kernel.o
	LD=$(LLVM_BIN)lld tools/build_tool/target/release/build_tool 0x40000000 0x0 $@ $^

analyze: build/Bootloader.bin
	$(ANALYZE) $(TARGET_32) build/Bootloader32.elf
	$(ANALYZE) $(TARGET_64) build/Bootloader64.elf
