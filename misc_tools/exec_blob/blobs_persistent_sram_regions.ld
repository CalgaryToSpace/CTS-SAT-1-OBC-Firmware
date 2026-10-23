/* blobs_persistent_sram_regions.ld

   Fixed SRAM regions used by blobs to persist small state blocks between
   their separate, self-rescheduled executions. INCLUDEd inside the MEMORY
   block of each blob's blob.ld, so that every blob sees every slot that's
   taken. Add new slots here; never overlap an existing one (both blobs may
   run at the same time).

   Rationale: This is real physical SRAM on the STM32L4R5 (640KB total,
   contiguous from 0x20000000 to 0x200A0000 across SRAM1+SRAM2+SRAM3 -- the
   "RAM2"/"RAM3" regions in the main firmware's linker script are aliases/
   subsets of this same physical bank, not separate memory).

   The slots are placed in the free tail above the reference rc3 ELF's .bss
   end (__bss_end__ / _end = 0x2008E370, confirmed via
   `arm-none-eabi-nm misc_tools/exec_blob/CTS-SAT-1_FW_rc3.elf`, which
   includes the 128KB static FreeRTOS heap `ucHeap`), and stop well below
   0x20098000, i.e., 32KB below _estack (0x200A0000), to leave headroom for
   the interrupt/MSP stack that continues to run at the top of RAM.

   These ranges do NOT intersect any address used by the currently-running
   rc3 firmware image's .data/.bss. They are raw, not-zero-initialized SRAM
   that physically survives across separate executions of a blob (each
   execution is freshly loaded into a different malloc'd/MPI buffer elsewhere
   and jumped into -- blobs have no .data/.bss of their own, per the
   Makefile's build-time check). */

/* gnss_bestxyzb_ring_blob (v1+): GNSS_ring_state_t. */
GNSS_RING (xrw) : ORIGIN = 0x2008F000, LENGTH = 0x100

/* 0x2008F100 - 0x2008F280: Deliberately unused, as growth room for GNSS_RING.
   GNSS_ring_state_t is currently 0x80 bytes, so 0x2008F080 - 0x2008F280
   (0x200 bytes, 4x its current usage) is kept free after it. */

/* extended_beacon_blob (v5+): BEACON_ring_state_t. */
BEACON_RING (xrw) : ORIGIN = 0x2008F280, LENGTH = 0x40

/* Next free address: 0x2008F2C0 */
