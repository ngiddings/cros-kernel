ENTRY(_start)

SECTIONS
{
    __high_mem = 0xFFFFFF8000000000;
    __load_addr = 0x80000;
    __begin = __high_mem + __load_addr;

    . = __load_addr; 
    .boot : 
    {
        KEEP(*(.boot.text))
        KEEP(*(.boot.data))
        src/aarch64/bootstrap.o
    }
    . = __high_mem + ALIGN(4096);

    __text_start = .;
    .text : AT(ADDR(.text) - __high_mem)
    {
        *(.text)
    }
    . = ALIGN(4096);
    __text_end = .;
 
    __rodata_start = .;
    .rodata : AT(ADDR(.rodata) - __high_mem)
    {
        *(.rodata)
    }
    . = ALIGN(4096);
    __rodata_end = .;
 
    __data_start = .;
    .data : AT(ADDR(.data) - __high_mem)
    {
        *(.data)
    }
    . = ALIGN(4096);
    __data_end = .;
 
    __bss_start = .;
    .bss : AT(ADDR(.bss) - __high_mem)
    {
        bss = .;
        *(.bss)
    }
    . = ALIGN(4096);
    __bss_end = .;
    __bss_size = __bss_end - __bss_start;
    __end = .;
}