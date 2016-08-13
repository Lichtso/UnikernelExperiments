HPP_SRC := $(wildcard src/*.hpp)
CPP_SRC := $(wildcard src/*.cpp)
ASM_SRC := $(wildcard src/*.s)
BIN := $(ASM_SRC:src/%.s=build/%.o) $(CPP_SRC:src/%.cpp=build/%.o)
COMPILE = $(LLVM_BIN)clang-3.9 -O1 -Iinclude -c -target armv7a-none-eabi -Wall -Wsign-compare

build/%.o : src/%.s
	$(COMPILE) -o $@ $<

build/%.o : src/%.cpp $(HPP_SRC)
	$(COMPILE) -fno-exceptions -fno-unwind-tables -fno-stack-protector -fno-rtti -ffreestanding -std=c++1z -o $@ $<

build/bootloader.bin: build/bootloader.elf ImageBuilder/ImageBuilder
	ImageBuilder/ImageBuilder $< $@

build/bootloader.elf : src/bootloader.lds $(BIN)
	$(LLVM_BIN)lld -flavor gnu -s --script $< -o $@ $(BIN)

analyze: build/bootloader.elf
	$(LLVM_BIN)llvm-objdump -d -s -t -print-imm-hex $<

ImageBuilder/ImageBuilder: ImageBuilder/main.cpp
	clang++ -IImageBuilder/elfio -o $@ $<
