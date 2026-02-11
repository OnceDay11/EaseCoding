ENTRY(no_main)

SECTIONS
{
    . = 0x400000 + SIZEOF_HEADERS;
    tinytest  : { *(.text) *(.data) *(.rodata .rodata.*) }
    /DISCARD/ : { *(.comment) *(.note.*) *(.eh_frame)}
}
