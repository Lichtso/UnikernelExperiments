    .section .entry,"ax"
_start:
    .global _start

    // http://www.heyrick.co.uk/armwiki/The_Status_register
    mrs r0, cpsr // read status register
    bic r0, r0, #0x001F // clear mode
    orr r0, r0, #0x0013 // set supervisor mode
    orr r0, r0, #0x00C0 // disable IRQ and FIQ interrupts
    bic r0, r0, #0x0200 // set little endian
    msr cpsr_c, r0 // write status register

    // http://infocenter.arm.com/help/topic/com.arm.doc.ddi0500e/BABCIJIA.html
    // mrc p15, 0, r0, c1, c0 // read system control register
    // bic r0, r0, #0x2000 // normal exception vectors (VBAR)
    // bic r0, r0, #0x0007 // disable MMU, alignment check, data cache
    // bic r0, r0, #0x1000 // disable instruction cache
    // mcr p15, 0, r0, c1, c0 // write system control register

    // ldr r0, =0x00017FF0
    // mov sp, r0
    // bl _Z4mainv

    // Write CPU0 AArch64 reset address
    ldr r1, =0x017000A0
    ldr r0, =0x00010200
    str r0, [r1]
    dsb
    isb

    // Request warm reset to AArch64
    mov r0, #3
    mcr p15, 0, r0, c12, c0, 2
    isb
loop:
	wfi
    b loop
