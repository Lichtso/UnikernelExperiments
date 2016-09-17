SECTIONS {
    . = 0x00010200;
    .entry : { *(.entry) }
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) }
    /DISCARD/ : { *(.ARM.exidx*) }
    /DISCARD/ : { *(.comment) }
}
