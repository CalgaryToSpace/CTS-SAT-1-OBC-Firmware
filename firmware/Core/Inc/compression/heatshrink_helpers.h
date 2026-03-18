#ifndef INCLUDE_GUARD__HEATSHRINK_HELPERS_H
#define INCLUDE_GUARD__HEATSHRINK_HELPERS_H

#include <stdint.h>
#include "littlefs/lfs.h"

int8_t LFS_compress_lfs_file_with_heatshrink(
    lfs_t *lfs,
    const char *input_path,
    const char *output_path
);

#endif // INCLUDE_GUARD__HEATSHRINK_HELPERS_H
