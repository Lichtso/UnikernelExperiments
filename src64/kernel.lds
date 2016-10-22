SECTIONS {
    . = 0x40000014;
    .entry : { *(.entry) }
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) }
    /DISCARD/ : { *(.ARM.exidx*) }
    /DISCARD/ : { *(.comment) }
}
