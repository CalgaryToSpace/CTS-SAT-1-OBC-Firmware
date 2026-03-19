#ifndef HEATSHRINK_CONFIG_H
#define HEATSHRINK_CONFIG_H

/* Should functionality assuming dynamic allocation be used? */
#ifndef HEATSHRINK_DYNAMIC_ALLOC
#define HEATSHRINK_DYNAMIC_ALLOC 1
#endif

#if HEATSHRINK_DYNAMIC_ALLOC
    /* Optional replacement of malloc/free */
    // #define HEATSHRINK_MALLOC(SZ) malloc(SZ)
    // #define HEATSHRINK_FREE(P, SZ) free(P)

    #include "FreeRTOS.h"
    #include <stddef.h>

    static inline void *heatshrink_port_impl_malloc(size_t size) {
        // Note: When investigated, the allocated size for the main operation (opening a file)
        // is 2048 bytes (LFS_CACHE_SIZE).
        // HEATSHRINK_debug_malloc_total_count++;
        void *ptr = pvPortMalloc(size);
        if (ptr == NULL) {
            // HEATSHRINK_debug_malloc_failed_count++;
        }
        return ptr;
    }

    static inline void heatshrink_port_impl_free(void *ptr) {
        // HEATSHRINK_debug_free_total_count++;
        vPortFree(ptr);
    }

    // Route allocations to FreeRTOS heap.
    #define HEATSHRINK_MALLOC(size)     heatshrink_port_impl_malloc(size)
    #define HEATSHRINK_FREE(ptr, size)  heatshrink_port_impl_free(ptr)

#else
    /* Required parameters for static configuration */
    #define HEATSHRINK_STATIC_INPUT_BUFFER_SIZE 32
    #define HEATSHRINK_STATIC_WINDOW_BITS 8
    #define HEATSHRINK_STATIC_LOOKAHEAD_BITS 4
#endif

/* Turn on logging for debugging. */
#define HEATSHRINK_DEBUGGING_LOGS 0

/* Use indexing for faster compression. (This requires additional space.) */
#define HEATSHRINK_USE_INDEX 1

#endif
