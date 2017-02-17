HPP_SRC := $(wildcard include/*.hpp) $(wildcard include/Hardware/*.hpp) $(wildcard include/Memory/*.hpp) $(wildcard include/Net/*.hpp)
LINK = $(LLVM_BIN)lld -flavor gnu -s -Linclude --script
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

build/Bootloader.bin: build/Bootloader32.elf build/Bootloader64.elf
	tools/target/debug/image_builder 0x10000 0x2000 $@ $?

build/Kernel.bin: build/Kernel.elf
	tools/target/debug/image_builder 0x40000000 0x0 $@ $?

build/Bootloader32.elf : src32/Bootloader.lds build/Entry32.o
	$(LINK) $< -o $@ build/Entry32.o

build/Bootloader64.elf : src64/Bootloader.lds build/Bootloader.o build/Entry64.o
	$(LINK) $< -o $@ build/Bootloader.o build/Entry64.o

build/Kernel.elf : src64/Kernel.lds build/Kernel.o build/ExceptionTable.o
	$(LINK) $< -o $@ build/Kernel.o

analyze: build/Entry32.o build/Entry64.o build/Bootloader.o build/Kernel.elf
	$(ANALYZE) $(TARGET_32) build/Entry32.o
	$(ANALYZE) $(TARGET_64) build/Entry64.o build/Bootloader.o build/Kernel.elf

tools/screen: tools/screen.c
	clang++ -o $@ $<
