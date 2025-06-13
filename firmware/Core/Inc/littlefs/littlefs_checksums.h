#ifndef INCLUDE_GUARD__LITTLEFS_CHECKSUMS_H
#define INCLUDE_GUARD__LITTLEFS_CHECKSUMS_H

#include <stdint.h>

int8_t LFS_read_file_checksum_sha256(
    const char filepath[], uint32_t start_offset, uint32_t max_length, uint8_t sha256_dest[32]
);

#endif /* INCLUDE_GUARD__LITTLEFS_CHECKSUMS_H */
