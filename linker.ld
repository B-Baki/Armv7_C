ENTRY(_start)

MEMORY
{
    ram (rwx) : ORIGIN = 0x00010000, LENGTH = 1M
}

SECTIONS
{
    .text : {
        *(.text*)
        *(.rodata*)
        *(.rodata.*)
    } > ram

    .data : {
        *(.data*)
        *(.data.*)
    } > ram

    .bss : {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss*)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > ram

    __stack_top = ORIGIN(ram) + LENGTH(ram);
}
