/* blob.ld */
ENTRY(blob_main)

MEMORY
{
    /* Reserved for this blob's persistent GNSS BESTXYZB ring buffer.
       This is real physical SRAM on the STM32L4R5 (640KB total, contiguous
       from 0x20000000 to 0x200A0000 across SRAM1+SRAM2+SRAM3 -- the "RAM2"/
       "RAM3" regions in the main firmware's linker script are aliases/
       subsets of this same physical bank, not separate memory).

       Placed in the free tail above the reference rc3 ELF's .bss end
       (__bss_end__ / _end = 0x2008E370, confirmed via
       `arm-none-eabi-nm misc_tools/exec_blob/CTS-SAT-1_FW_rc3.elf`, which
       includes the 128KB static FreeRTOS heap `ucHeap`), and stopping
       0x8000 (32KB) below _estack (0x200A0000) to leave headroom for the
       interrupt/MSP stack that continues to run at the top of RAM.

       This range does NOT intersect any address used by the currently-
       running rc3 firmware image's .data/.bss. It is raw, not-zero-
       initialized SRAM that happens to physically survive across separate
       executions of this blob (each execution is freshly loaded into a
       different malloc'd/MPI buffer elsewhere and jumped into -- this blob
       has no .data/.bss of its own, per the Makefile's build-time check).
       We use this fixed physical range as the *only* persistent storage
       available to this blob, so sampled GNSS data survives between the
       blob's repeated, self-rescheduled executions. */
    RING (xrw) : ORIGIN = 0x2008F000, LENGTH = 0x9000

    /* Wide-open region for the blob's own code/rodata (position-independent,
       loaded at a runtime-chosen malloc'd/MPI-buffer address -- this region
       just needs to be large enough and not overlap RING; it is NOT where
       the code actually ends up executing from). Defining any MEMORY region
       at all makes every output section (including ones spliced in via
       "INSERT AFTER" below) require an explicit region, so .text/.got must
       be assigned one explicitly here, otherwise they get incorrectly
       squeezed into RING (which is far too small for them). */
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
   the `gnss_ring_buffer_base` symbol at RING's origin. Marked NOLOAD so
   objcopy -O binary emits zero bytes for it -- it reserves no space in
   blob.bin, and is never written into by the blob loader. */
SECTIONS
{
    .gnss_ring_buffer (NOLOAD) : {
        gnss_ring_buffer_base = .;
        . = . + 0x9000;
    } > RING
}
