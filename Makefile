HPP_SRC := $(wildcard include/*.hpp) $(wildcard include/Hardware/*.hpp) $(wildcard include/Net/*.hpp)
ANALYZE = $(LLVM_BIN)llvm-objdump -print-imm-hex -d -s -t -triple
COMPILE = $(LLVM_BIN)clang-3.9 -O1 -Iinclude -c -mlittle-endian -Wall -Wsign-compare -target
COMPILE_CPP = -fno-exceptions -fno-unwind-tables -fno-stack-protector -fno-rtti -ffreestanding -std=c++1z
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

build/bootloader.bin: tools/ImageBuilder build/bootloader32.elf build/bootloader64.elf
	$< $@ 0x10000 0x2000 build/bootloader32.elf build/bootloader64.elf

build/bootloader32.elf : src32/bootloader.lds build/entry32.o
	$(LLVM_BIN)lld -flavor gnu -s --script $< -o $@ build/entry32.o

build/bootloader64.elf : src64/bootloader.lds build/bootloader.o build/entry64.o
	$(LLVM_BIN)lld -flavor gnu -s --script $< -o $@ build/bootloader.o build/entry64.o

analyze: build/bootloader32.elf build/bootloader64.elf
	$(ANALYZE) $(TARGET_32) build/bootloader32.elf
	$(ANALYZE) $(TARGET_64) build/bootloader64.elf

tools/ImageBuilder: tools/ImageBuilder.cpp
	clang++ -Itools/elfio -o $@ $<

tools/screen: tools/screen.cpp
	clang++ -o $@ $<
