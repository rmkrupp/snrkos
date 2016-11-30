/* kernel.ld - kernel m32 linker file
 *
 * Author: Rebecca Krupp (beka.krupp@gmail.com)
 */

OUTPUT_ARCH(i386)

ENTRY(_start)

SECTIONS
{
    . = 0x100000;

    .low_mem :
    {
        *(.multiboot)
    }

    .text BLOCK(4096) :
    {
        *(.text)
        *(.rodata*)
    }

    .data BLOCK(4096) :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
        *(COMMON*)
    }

    _end = .;

    /DISCARD/ :
    {
        *(.comment)
    }
}
