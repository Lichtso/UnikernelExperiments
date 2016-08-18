    .thumb_func
    .thumb
    .global dump
    .type dump, function
    .global dumpSection
    .type dumpSection, function
    .global putAddr32
    .type putAddr32, function
    .global putHex32
    .type putHex32, function
    .global putHex4
    .type putHex4, function
    .global putChar
    .type putChar, function

dump:
    push {lr}
    ldr r0, =0x00000000
    bl dumpSection
    ldr r0, =0x01C20000
    bl dumpSection
    ldr r0, =0x01C60000
    bl dumpSection
    pop {pc}

dumpSection:
    push {r1, lr}
    mov r1, r0
    add r2, r0, #0x10000
dumpSectionLoop:
    mov r0, r1
    bl putAddr32
    add r1, r1, #4
    cmp r2, r1
    bne dumpSectionLoop
    pop {r1, pc}

putAddr32:
    push {r1, lr}
    mov r1, r0
    bl putHex32
    mov r0, #32
    bl putChar
    ldr r0, [r1]
    bl putHex32
    mov r0, #10
    bl putChar
    pop {r1, pc}

putHex32:
    push {r1, lr}
    mov r1, r0
    ubfx r0, r1, #28, #4
    bl putHex4
    ubfx r0, r1, #24, #4
    bl putHex4
    ubfx r0, r1, #20, #4
    bl putHex4
    ubfx r0, r1, #16, #4
    bl putHex4
    ubfx r0, r1, #12, #4
    bl putHex4
    ubfx r0, r1, #8, #4
    bl putHex4
    ubfx r0, r1, #4, #4
    bl putHex4
    ubfx r0, r1, #0, #4
    bl putHex4
    pop {r1, pc}

putHex4:
    push {lr}
    cmp r0, 10
    ite lt
    addlt r0, r0, #48
    addge r0, r0, #55
    bl putChar
    pop {pc}

putChar:
    push {r1, r2, lr}
    ldr r1, =0x01C28014
    putCharLoop:
    ldr r2, [r1]
    tst r2, #0x20
    beq putCharLoop
    ldr r1, =0x01C28000
    strb r0, [r1]
    pop {r1, r2, pc}
