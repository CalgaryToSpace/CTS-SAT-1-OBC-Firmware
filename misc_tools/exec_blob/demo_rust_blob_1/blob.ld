/* blob.ld */
ENTRY(blob_main)

SECTIONS
{
    . = 0x00000000;

    .text : {
        *(.text.entry)   /* blob_main forced to offset 0 */
        *(.text*)
        *(.rodata*)
    }

    /DISCARD/ : {
        *(.ARM.exidx*)
        *(.ARM.extab*)
    }
}
