    .section .entry,"ax"
_start:
    .global _start

    // Setup stack pointer: 64 KiB space in SRAM A2
    ldr x0, =0x00054000
    mov sp, x0

    // Enable advanced SIMD and floating-point execution
    mov x0, #3<<20
    msr CPACR_EL1, x0

    // Start main execution
    bl _Z4mainv

    // Bumper to stop execution
    wfi
