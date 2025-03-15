#ifndef INCLUDE_GUARD__LITTLEFS_BENCHMARK_H__
#define INCLUDE_GUARD__LITTLEFS_BENCHMARK_H__

/*-----------------------------INCLUDES----------------------------------------*/
#include <stdint.h>

/*-------------------------------BENCHMARK FEATURES----------------------------*/
typedef enum {
    LFS_NEW_FILE,
    LFS_APPEND_FILE
} LFS_benchmark_write_mode_enum_t;

typedef enum {
    LFS_MOUNT_EACH,
    LFS_MOUNT_ONCE
} LFS_benchmark_mount_mode_enum_t;

/*-----------------------------BENCHMARK FUNCTIONS-----------------------------*/
uint8_t LFS_benchmark_write_read(uint16_t write_size, uint16_t write_count, char* response_str, uint16_t response_str_len, LFS_benchmark_write_mode_enum_t write_mode, LFS_benchmark_mount_mode_enum_t mount_mode);
uint8_t LFS_benchmark_write_read_single_and_new(uint16_t write_chunk_size, uint16_t write_chunk_count, char* response_str, uint16_t response_str_len);

#endif // INCLUDE_GUARD__LITTLEFS_BENCHMARK_H__
