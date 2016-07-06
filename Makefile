build/kernel.elf : build/kernel.o src/linkerScript.txt
	$(LLVM_BIN)/lld -flavor gnu --script src/linkerScript.txt -o $@ $<

build/kernel.o : src/kernel.c
	$(LLVM_BIN)/clang-3.9 -fPIC -ffreestanding -c -target arm64 -o $@ $<

run:
	qemu-system-aarch64 -machine versatilepb -cpu cortex-a53 -serial stdio -kernel build/kernel.elf
