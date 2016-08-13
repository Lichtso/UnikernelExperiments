entry:
    .global entry

    ldr sp, =0x00017FF0
    bl _Z4mainv
    wfi
