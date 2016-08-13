SECTIONS {
    . = 0x00010338;
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) }
    /DISCARD/ : { *(.ARM.exidx*) }
    /DISCARD/ : { *(.comment) }
}

ENTRY(entry)
