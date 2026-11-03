/* blob.ld */
ENTRY(blob_main)

MEMORY
{
    /* Persistent SRAM regions shared by all blobs (defines GNSS_RING, used
       below for this blob's small persistent state block: write cursor,
       flags, counters -- the sampled GNSS data itself lives in LittleFS, not
       here; see GNSS_ring_state_t). See the included file for the rationale. */
    INCLUDE ../blobs_persistent_sram_regions.ld

    /* Wide-open region for the blob's own code/rodata (position-independent,
       loaded at a runtime-chosen malloc'd/MPI-buffer address -- this region
       just needs to be large enough and not overlap GNSS_RING; it is NOT where
       the code actually ends up executing from). Defining any MEMORY region
       at all makes every output section (including ones spliced in via
       "INSERT AFTER" below) require an explicit region, so .text/.got must
       be assigned one explicitly here, otherwise they get incorrectly
       squeezed into GNSS_RING (which is far too small for them). */
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

/* Separate SECTIONS block (not part of the "INSERT AFTER" flow above, since
   it targets a completely different, fixed-address memory region): anchors
   the `gnss_ring_state_base` symbol at GNSS_RING's origin. Marked NOLOAD so
   objcopy -O binary emits zero bytes for it -- it reserves no space in
   blob.bin, and is never written into by the blob loader. */
SECTIONS
{
    .gnss_ring_state (NOLOAD) : {
        gnss_ring_state_base = .;
        . = . + 0x100;
    } > GNSS_RING
}
