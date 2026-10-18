/* User programs run at the start of the user range (USER_START in src/mm/layout.h). */
ENTRY(_start)

SECTIONS {
    . = 0x40000000;

    .text : {
        *(.text.entry)
        *(.text*)
    }

    .rodata : { *(.rodata*) }

    /* Keep .bss inside .data so the flat binary contains it, already zeroed. */
    .data : {
        *(.data*)
        *(COMMON)
        *(.bss*)
    }

    /DISCARD/ : { *(.note*) *(.comment) *(.eh_frame*) }
}
