# FreeRTOS and Memory (Stack and Heap)

* The C stdlib heap used by `malloc()` (which is entirely unused) is different than the FreeRTOS heap (which is used for task stacks, and other things).
    * The linker script's `_Min_Heap_Size` value refers to this stdlib heap, and not the FreeRTOS heap.
* The FreeRTOS heap is configured in the IOC file, and in `Core/Inc/FreeRTOSConfig.h`. Its size is set, in bytes, in `configTOTAL_HEAP_SIZE`. 
* Currently, `configTOTAL_HEAP_SIZE` is set to 65536 bytes.
* The STM32 has 640 KiB of contiguous SRAM, split across three regions. The linker script maps all 640 KiB into one region so that we can use it all. Entirely valid approach, as far as I can tell. The default generated linker script sets 3 smaller RAM regions, then expects C code to target the regions. Largely unnecessary, as far as I can tell.
* The FreeRTOS heap is allocated as a giant static byte array as such: `static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];`
    * Thus, this array is accounted for in the RAM line of the linker output (which shows total memory usage).
* A reasonable threshold for validation is to ensure that at least 2048-4096 bytes are unused in the RAM memory region. We don't need much more than that, because task stacks are all placed in the FreeRTOS heap - it's just the FreeRTOS overhead/internals whose "real" stack gets built in that free space.
* FreeRTOS heap allocation is done with `pvPortCalloc`/`pvPortMalloc`. Easy to audit/search for uses.

```
Memory region         Used Size  Region Size  %age Used
             RAM:      600104 B       640 KB     91.57%
```

## Heap Allocation

* In general, we avoid heap allocation on embedded systems. Easy to make errors with it, makes code tougher to audit, non-constant allocation time can be challenging, etc.
* On CTS-SAT-1, we use heap allocation in the following places:
    * FreeRTOS, for the task stacks
    * LittleFS, when opening a file
    * `exec_blob_from_fs` telecommand, with certain argument
* We must ensure the size of the FreeRTOS heap, allocated in the BSS RAM segment as a global byte array, is less than the sum of the stacks, and all other expected heap allocation uses.
