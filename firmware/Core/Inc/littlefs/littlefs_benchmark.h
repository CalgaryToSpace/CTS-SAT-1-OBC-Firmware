#ifndef __INCLUDE_GUARD__LITTLEFS_BENCHMARK_H__
#define __INCLUDE_GUARD__LITTLEFS_BENCHMARK_H__

/*-----------------------------INCLUDES----------------------------------------*/
#include <stdint.h>

/*-------------------------------BENCHMARK FEATURES----------------------------*/
typedef enum {
    LFS_SINGLE_FILE,
    LFS_NEW_FILE
} LFS_benchmark_mode_enum_t;

/*-----------------------------BENCHMARK FUNCTIONS-----------------------------*/
uint8_t LFS_benchmark_write_read(uint16_t write_chunk_size, uint16_t write_chunk_count, char* response_str, uint16_t response_str_len, LFS_benchmark_mode_enum_t mode);
uint8_t LFS_benchmark_write_read_single_and_new(uint16_t write_chunk_size, uint16_t write_chunk_count, char* response_str, uint16_t response_str_len);

#endif // __INCLUDE_GUARD__LITTLEFS_BENCHMARK_H__
