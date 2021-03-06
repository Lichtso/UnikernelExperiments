# UnikernelExperiments
TCP/IPv6-enabled [Pine64+](https://www.pine64.org/?product=pine-a64-board-2gb) bootloader

This project provides you with the toolchain and resources needed to start developing your own unikernel.
It is completely written from scratch and is thus still well structured without legacy code.


## Getting started

You will need:
- Make
- LLVM 3.9 or higher: [Clang](http://clang.llvm.org/get_started.html), [LLD](http://lld.llvm.org), llvm-objdump (optional)
- [Rust](https://www.rust-lang.org/en-US/)
- Pine64+ with 2GB (other versions are untested)
- Mirco-SD-card to store the bootloader
- Micro-USB-cable for power supply
- RJ45-cable for data transfer
- UART for debugging (optional)

```bash
make build/Bootloader.bin
dd if=build/Bootloader.bin of=/dev/[Mirco-SD-Card]
tools/screen /dev/[UART-USB]
ping6 FE80::34C9:E3FF:FEF1:B805%[Interface]
make build/Kernel.bin
nc FE80::34C9:E3FF:FEF1:B805%[Interface] 1337 < build/Kernel.bin
```


## Example Use Cases

- (Self)-Education: Learn or teach how low level software works
- Experimental development: Develop your own unikernel easily
- Internet of Things: Leave behind all the bloatware
- Security: Less code -> less breaches


## State of Affairs / Features

- Hardware Driver
    - UART ✓
    - RSB ✓
    - AXP803 ✓
    - DRAM
        - 2GB ✓
        - Auto size detection
    - Ethernet ✓
    - High Speed Timer ✓
    - Timer
    - Real Time Clock
    - Interrupt Controller
    - SD-Card
    - Thermal Sensors
    - DVFS

- Software Driver
    - 64 Bit: ARMv8 / ARM64 / AArch64 ✓
    - Floating point unit ✓
    - Caches ✓
    - MMU
    - Interrupt Handling
    - Multi threading
    - IPv4 / ICMPv4
    - IPv6 / ICMPv6, implemented features:
        - Echo ✓
        - Neighbor solicitation and advertisement ✓
    - UDP ✓
    - TCP
        - IPv4
        - IPv6 ✓
        - Receiving payload ✓
        - Sending payload
        - Connect / Listen / Close ✓
        - Sequence number overflow
        - Receive and transmit ring buffer
        - Timestamps
        - Selective acknowledgment
        - Initial sequence numbers
        - Correct timings
        - Multiple connections


## Communication Interfaces

UART is a nice and easy interface for debugging and as a interactive console.
But it is too slow to upload large binaries.
The firmware comes with a USB-OTG boot option but:
- The protocol is complicated and badly documented
- Ethernet can be faster: Up to 1 GBit/s instead of 0.48 GBits/s of USB2
- RJ45-cables are more common than USB-OTG-TypeA-cables
- You probably want to connect the board to a network anyway and thus don't need a extra USB connection only for booting


## References

### Wiki
- http://linux-sunxi.org/Arm64
- http://linux-sunxi.org/Pine64
- http://linux-sunxi.org/BROM
- http://linux-sunxi.org/FEL/USBBoot
- http://linux-sunxi.org/Reduced_Serial_Bus
- https://sourceware.org/binutils/docs/ld/Scripts.html
- https://sourceware.org/binutils/docs/as/ARM-Directives.html
- http://www.heyrick.co.uk/armwiki/The_Status_register
- http://downloads.ti.com/docs/esd/SPNU118N/Content/SPNU118N_HTML/assembler_directives.html
- https://developer.arm.com/docs/den0024/latest/12-the-memory-management-unit/124-translation-tables-in-armv8-a/1243-cache-configuration
- https://en.wikipedia.org/wiki/Cache

### Tutorials
- https://balau82.wordpress.com/2010/04/12/booting-linux-with-u-boot-on-qemu-arm/
- https://balau82.wordpress.com/2010/02/28/hello-world-for-bare-metal-arm-using-qemu/
- https://singpolyma.net/2012/01/writing-a-simple-os-kernel-part-1/

### Code
- https://github.com/allwinner-zh/bootloader
- https://github.com/linux-sunxi/sunxi-tools/blob/master/uart0-helloworld-sdboot.c
- https://github.com/apritzel/linux/commits/a64-wip
- https://github.com/longsleep/linux-pine64/tree/pine64-hacks-2.0-experimental
- https://github.com/torvalds/linux/blob/master/arch/arm64/kernel/setup.c
- https://github.com/torvalds/linux/blob/master/arch/arm64/mm/cache.S
- https://github.com/torvalds/linux/blob/master/arch/arm64/mm/proc.S

### Docs
- http://infocenter.arm.com/help/topic/com.arm.doc.den0024a/index.html
- http://dl.linux-sunxi.org/A64/A64_Datasheet_V1.1.pdf
- http://linux-sunxi.org/images/4/4b/Allwinner_H3_Datasheet_V1.2.pdf
- http://www.cl.cam.ac.uk/research/srg/han/ACS-P35/zynq/arm_gic_architecture_specification.pdf
- http://infocenter.arm.com/help/topic/com.arm.doc.ddi0471a/DDI0471A_gic400_r0p0_trm.pdf
- http://files.pine64.org/doc/datasheet/pine64/AXP803_Datasheet_V1.0.pdf
- http://download3.dvd-driver.cz/realtek/datasheets/pdf/rtl8211e(g)-vb(vl)-cg_datasheet_1.6.pdf
