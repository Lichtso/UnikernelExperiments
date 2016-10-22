    .section .entry,"ax"
_start:
    .global _start

    // Activate instruction cache
    mrs x0, SCTLR_EL3
    orr x0, x0, #(1<<12)
    msr SCTLR_EL3, x0
    ic iallu // Invalidate all instruction cache lines

    // Setup stack pointer: 64 KiB space in SRAM A2
    ldr x0, =0x00054000
    mov sp, x0

    bl _Z4mainv // Start main execution
    wfi // Bumper to stop execution
