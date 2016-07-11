CPP_SRC := $(wildcard src/*.cpp)
ASM_SRC := $(wildcard src/*.s)
BIN := $(ASM_SRC:src/%.s=build/%.o) $(CPP_SRC:src/%.cpp=build/%.o)
COMPILE = $(LLVM_BIN)clang-3.9 -O0 -c -target arm64 -Wall -Wsign-compare

run: build/kernel.elf
	qemu-system-aarch64 -machine versatilepb -cpu cortex-a53 -serial stdio -monitor telnet:localhost:1234,server,nowait -kernel $<

analyze: build/kernel.elf
	$(LLVM_BIN)llvm-objdump -d -s -t -print-imm-hex $<

build/kernel.elf : $(BIN) src/linkerScript.txt
	$(LLVM_BIN)lld -flavor gnu --script src/linkerScript.txt -o $@ $(BIN)

build/%.o : src/%.cpp
	$(COMPILE) -fno-exceptions -fno-stack-protector -fno-rtti -ffreestanding -std=c++1z -o $@ $<

build/%.o : src/%.s
	$(COMPILE) -o $@ $<
