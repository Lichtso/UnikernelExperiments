    .section .entry,"ax"
_start:
    .global _start

    // Setup stack pointer: 64 KiB space in SRAM A2
    ldr x0, =0x00054000
    mov sp, x0

    bl _Z4mainv // Start main execution
    wfi // Bumper to stop execution
