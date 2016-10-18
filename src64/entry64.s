    .section .entry,"ax"
_start:
    .global _start

    // Activate instruction cache
    mrs x0, SCTLR_EL3
    orr x0, x0, #(1<<12)
    msr SCTLR_EL3, x0
    ic iallu // Invalidate all instruction cache lines

    ldr x0, =0x00017FF0
    mov sp, x0
    bl _Z4mainv
    wfi
