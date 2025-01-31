#ifndef INCLUDE_GUARD__FLASH_BENCHMARK_H__
#define INCLUDE_GUARD__FLASH_BENCHMARK_H__

#include <stdint.h>

uint8_t FLASH_benchmark_erase_write_read(uint8_t chip_num, uint32_t test_data_address, uint16_t test_data_length, char* response_str, uint16_t response_str_len);

#endif /* INCLUDE_GUARD__FLASH_BENCHMARK_H__ */
