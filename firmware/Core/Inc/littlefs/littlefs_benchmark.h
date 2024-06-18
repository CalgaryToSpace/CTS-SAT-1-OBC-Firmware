#ifndef __INCLUDE_GUARD__LITTLEFS_BENCHMARK_H__
#define __INCLUDE_GUARD__LITTLEFS_BENCHMARK_H__

#include <stdint.h>


uint8_t LFS_benchmark_write_read(uint16_t write_chunk_size, uint16_t write_chunk_count, char* response_str, uint16_t response_str_len);

#endif // __INCLUDE_GUARD__LITTLEFS_BENCHMARK_H__
