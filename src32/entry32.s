    .section .entry,"ax"
_start:
    .global _start

    mrs r0, cpsr // Read status register
    bic r0, r0, #0x001F // Clear mode
    orr r0, r0, #0x0013 // Set supervisor mode
    orr r0, r0, #0x00C0 // Disable IRQ and FIQ interrupts
    bic r0, r0, #0x0200 // Set little endian
    msr cpsr_c, r0 // Write status register

    // Write CPU0 AArch64 reset address
    ldr r1, =0x017000A0
    ldr r0, =0x00010050
    str r0, [r1]

    // Request warm reset to AArch64
    mov r0, #3
    mcr p15, 0, r0, c12, c0, 2
    wfi
