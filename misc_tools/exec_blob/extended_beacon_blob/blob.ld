/* blob.ld */
ENTRY(blob_main)

MEMORY
{
    /* Persistent SRAM regions shared by all blobs (defines BEACON_RING, used
       below for this blob's small persistent state block: the beacon ring
       write cursor and counters -- the stored beacons themselves live in
       LittleFS; see BEACON_ring_state_t). See the included file for the
       rationale. */
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

    /* Placed explicitly (contiguous with .text) so it doesn't fall through to
       the default shared-object script's orphan-section placement, which
       page-aligns .got and leaves a multi-KB gap that objcopy zero-fills. */
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

/* Anchors the `beacon_ring_state_base` symbol at BEACON_RING's origin. NOLOAD,
   so it occupies zero bytes in blob.bin. */
SECTIONS
{
    .beacon_ring_state (NOLOAD) : {
        beacon_ring_state_base = .;
        . = . + 0x100;
    } > BEACON_RING
}
