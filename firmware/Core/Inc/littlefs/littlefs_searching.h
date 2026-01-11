#ifndef INCLUDE_GUARD__LITTLEFS_SEARCHING_H
#define INCLUDE_GUARD__LITTLEFS_SEARCHING_H

#include <stdint.h>
#include <stdlib.h>
#include "littlefs/lfs.h"


int32_t LFS_search_count_occurrences(
    lfs_t *lfs,
    lfs_file_t *file,
    const uint8_t *needle,
    size_t needle_len
);


int32_t LFS_search_find_nth_occurrence(
    lfs_t *lfs,
    lfs_file_t *file,
    const uint8_t *needle,
    size_t needle_len,
    uint16_t n,
    lfs_soff_t *out_offset
);

#endif // INCLUDE_GUARD__LITTLEFS_SEARCHING_H
