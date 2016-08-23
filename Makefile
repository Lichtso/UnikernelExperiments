HPP_SRC := $(wildcard include/*.hpp) $(wildcard include/Hardware/*.hpp) $(wildcard include/Net/*.hpp)
CPP_SRC := $(wildcard src/*.cpp)
ASM_SRC := $(wildcard src/*.s)
BIN := $(ASM_SRC:src/%.s=build/%.o) $(CPP_SRC:src/%.cpp=build/%.o)
COMPILE = $(LLVM_BIN)clang-3.9 -O1 -Iinclude -c -target armv7a-none-eabi -mlittle-endian -Wall -Wsign-compare

build/%.o : src/%.s
	$(COMPILE) -o $@ $<

build/%.o : src/%.cpp $(HPP_SRC)
	$(COMPILE) -fno-exceptions -fno-unwind-tables -fno-stack-protector -fno-rtti -ffreestanding -std=c++1z -o $@ $<

build/bootloader.bin: build/bootloader.elf tools/ImageBuilder
	tools/ImageBuilder $< $@

build/bootloader.elf : src/bootloader.lds $(BIN)
	$(LLVM_BIN)lld -flavor gnu -s --script $< -o $@ $(BIN)

analyze: build/bootloader.elf
	$(LLVM_BIN)llvm-objdump -d -s -t -print-imm-hex $<

tools/ImageBuilder: tools/ImageBuilder.cpp
	clang++ -Itools/elfio -o $@ $<

tools/screen: tools/screen.cpp
	clang++ -o $@ $<
