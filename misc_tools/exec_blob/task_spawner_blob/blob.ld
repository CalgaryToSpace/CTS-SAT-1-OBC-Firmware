/* blob.ld */
ENTRY(blob_main)

MEMORY
{
    /* Persistent SRAM regions shared by all blobs (defines BLOB_TASKS, used
       below for the spawner's per-slot pointers). See the included file. */
    INCLUDE ../blobs_persistent_sram_regions.ld

    /* Wide-open region for the blob's own (position-independent) code/rodata.
       Defining any MEMORY region at all makes every output section require an
       explicit region, so .text/.got are assigned to this one explicitly. */
    BLOBMEM (rwx) : ORIGIN = 0x00000000, LENGTH = 0x10000000
}

SECTIONS
{
    . = 0x00000000;

    .text : {
        KEEP(*(.text.entry))   /* blob_main forced to offset 0 */
        *(.text*)
        *(.rodata*)
    } > BLOBMEM

    /* Placed explicitly (contiguous with .text), so there's no page-aligned gap. */
    .got : {
        *(.got.plt)
        *(.got)
    } > BLOBMEM

    /DISCARD/ : {
        *(.ARM.exidx*)
        *(.ARM.extab*)
    }
}
INSERT AFTER .text;

/* Anchors `blob_task_slots_base` at BLOB_TASKS's origin. NOLOAD, so it occupies
   zero bytes in blob.bin. */
SECTIONS
{
    .blob_task_slots (NOLOAD) : {
        blob_task_slots_base = .;
        . = . + 0x40;
    } > BLOB_TASKS
}
