HPP_SRC := $(wildcard include/*.hpp) $(wildcard include/Hardware/*.hpp) $(wildcard include/Net/*.hpp)
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

build/bootloader.bin: tools/ImageBuilder build/bootloader32.elf build/bootloader64.elf
	$< $@ 0x10000 0x2000 build/bootloader32.elf build/bootloader64.elf

build/kernel.bin: tools/ImageBuilder build/kernel.elf
	$< $@ 0x40000000 0x0 build/kernel.elf

build/bootloader32.elf : src32/bootloader.lds build/entry32.o
	$(LINK) $< -o $@ build/entry32.o

build/bootloader64.elf : src64/bootloader.lds build/bootloader.o build/entry64.o
	$(LINK) $< -o $@ build/bootloader.o build/entry64.o

build/kernel.elf : src64/kernel.lds build/kernel.o
	$(LINK) $< -o $@ build/kernel.o

analyze: build/entry32.o build/entry64.o build/bootloader.o build/kernel.o
	$(ANALYZE) $(TARGET_32) build/entry32.o
	$(ANALYZE) $(TARGET_64) build/entry64.o build/bootloader.o build/kernel.elf

tools/ImageBuilder: tools/ImageBuilder.cpp
	clang++ -Itools -o $@ $<

tools/screen: tools/screen.cpp
	clang++ -o $@ $<
