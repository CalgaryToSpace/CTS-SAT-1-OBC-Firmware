/* blob.ld */
ENTRY(blob_main)

SECTIONS
{
    . = 0x00000000;

    .text : {
        KEEP(*(.text.entry))   /* blob_main forced to offset 0 */
        *(.text*)
        *(.rodata*)
    }

    /* Placed explicitly (contiguous with .text) so it doesn't fall through to
       the default shared-object script's orphan-section placement, which
       page-aligns .got and leaves a multi-KB gap that objcopy zero-fills. */
    .got : {
        *(.got.plt)
        *(.got)
    }

    /DISCARD/ : {
        *(.ARM.exidx*)
        *(.ARM.extab*)
    }
}
INSERT AFTER .text;
