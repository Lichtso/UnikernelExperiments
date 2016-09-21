    .section .entry,"ax"
_start:
    .global _start

    // PSTATE
    // msr SPSel, #0 // SP_EL0
    // msr DAIFClr, #15

    // mrs x0, SCTLR_EL3
    // orr x0, x0, #(1<<12) // Activate Instruction Cache
    // msr SCTLR_EL3, x0
    // isb
    // and x0, x0, #~7 // Deactivate MMU, Alignment Check and Data Cache
    // orr x0, x0, #8 // Activate stack alignment
    // and x0, x0, #~(1<<25) // Set to little-endian
    // and x0, x0, #~(1<<19) // Deactivate write-xor-execute enforcement
    // msr SCTLR_EL3, x0

    // Invalidate icache and TLB for good measure
    // ic iallu
    // tlbi alle3
    // dsb sy
    // isb

    ldr x0, =0x00017FF0
    mov sp, x0
    bl _Z4mainv
    wfi
