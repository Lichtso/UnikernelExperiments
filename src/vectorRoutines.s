resetVector:
    .global resetVector
    ldr x1, =0x08000000
    mov sp, x1
    b main
    .space 0xF4



.macro saveContext
    stp x30, x29, [sp, #-16]!
    stp x28, x27, [sp, #-16]!
    stp x26, x25, [sp, #-16]!
    stp x24, x23, [sp, #-16]!
    stp x22, x21, [sp, #-16]!
    stp x20, x19, [sp, #-16]!
    stp x18, x17, [sp, #-16]!
    stp x16, x15, [sp, #-16]!
    stp x14, x13, [sp, #-16]!
    stp x12, x11, [sp, #-16]!
    stp x10,  x9, [sp, #-16]!
    stp  x8,  x7, [sp, #-16]!
    stp  x6,  x5, [sp, #-16]!
    stp  x4,  x3, [sp, #-16]!
    stp  x2,  x1, [sp, #-16]!
    str  x0,      [sp,  #-8]!
.endm

.macro restoreContext
    ldr  x0,      [sp],  #8
    ldp  x2,  x1, [sp], #16
    ldp  x4,  x3, [sp], #16
    ldp  x6,  x5, [sp], #16
    ldp  x8,  x7, [sp], #16
    ldp x10,  x9, [sp], #16
    ldp x12, x11, [sp], #16
    ldp x14, x13, [sp], #16
    ldp x16, x15, [sp], #16
    ldp x18, x17, [sp], #16
    ldp x20, x19, [sp], #16
    ldp x22, x21, [sp], #16
    ldp x24, x23, [sp], #16
    ldp x26, x25, [sp], #16
    ldp x28, x27, [sp], #16
    ldp x30, x29, [sp], #16
.endm

interruptVector:
    .global interruptVector

// Current EL SP0 Synchronous
    .space 0x80

// Current EL SP0 IRQ
    .space 0x80

// Current EL SP0 FIQ
    .space 0x80

// Current EL SP0 SError
    .space 0x80

// Current EL SPX Synchronous
    .space 0x80

// Current EL SPX IRQ
    saveContext
    ldr w28, #24
    ldr w27, [x28]
    mov fp, sp
    blr x27
    str wzr, [x28]
    b interruptReturn
    .word 0x10140030
    .space 0x24

// Current EL SPX FIQ
    .space 0x80

// Current EL SPX SError
    .space 0x80

// Lower EL AArch64 Synchronous
    .space 0x80

// Lower EL AArch64 IRQ
    .space 0x80

// Lower EL AArch64 FIQ
    .space 0x80

// Lower EL AArch64 SError
    .space 0x80

// Lower EL AArch32 Synchronous
    .space 0x80

// Lower EL AArch32 IRQ
    .space 0x80

// Lower EL AArch32 FIQ
    .space 0x80

// Lower EL AArch32 SError
    .space 0x80

interruptReturn:
    restoreContext
    eret
