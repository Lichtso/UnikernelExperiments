# UnikernelExperiments
Bootloader / unikernel for the Pine64+


## Status

- QEMU Versatile Board Experiments ✓
- Hello-World for Allwinner-A64 ✓

- Bootloader for Pine64+ [WIP]

- Software Driver
    - 64 Bit: ARMv8 / ARM64 / AArch64
    - Interrupt Handling
    - IPv4 / ICMPv4
    - IPv6 / ICMPv6 [WIP]
    - TCP
    - UDP

- Hardware Driver
    - UART ✓
    - RSB ✓
    - AXP803 ✓
    - DRAM ✓
    - Ethernet ✓
    - Interrupt Controller
    - SD-Card


## Aim of the Project

There are some similarities to other unikernels like [IncludeOS](https://github.com/hioa-cs/IncludeOS) in terms of:
- security
- latency / realtime
- speed
- energy consumption
- memory used

But this project does not aim to be POSIX or UNIX compatible, instead it is absolutely minimalistic.


## Example Use Cases

- Education
- Experimental development
- Internet of Things
