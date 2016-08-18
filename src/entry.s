entry:
    .global entry

    mrs r0, cpsr // read status register
    bic r0, r0, #0x001F // clear mode
    orr r0, r0, #0x0013 // set supervisor mode
    orr r0, r0, #0x00C0 // disable IRQ and FIQ interrupts
    bic r0, r0, #0x0200 // set little endian
    msr cpsr_c, r0 // write status register

    mrc p15, 0, r0, c1, c0 // read system control register
    bic r0, r0, #0x2000 // normal exception vectors (VBAR)
    bic r0, r0, #0x0007 // disable MMU, alignment check, data cache
    orr r0, r0, #0x0800 // ??? enable flow prediction ???
    bic r0, r0, #0x1000 // disable instruction cache
    mcr p15, 0, r0, c1, c0 // write system control register

    ldr sp, =0x00017FF0
    bl _Z4mainv
    wfi
