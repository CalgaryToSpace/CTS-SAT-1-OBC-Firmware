#ifndef __INCLUDE_GUARD__TELECOMMAND_ARGS_HELPERS_H__
#define __INCLUDE_GUARD__TELECOMMAND_ARGS_HELPERS_H__

#include <stdint.h>

uint8_t TCMD_ascii_to_uint64(const char *str, uint32_t str_len, uint64_t *result);
uint8_t TCMD_extract_uint64_arg(const char *str, uint32_t str_len, uint8_t arg_index, uint64_t *result);
uint8_t TCMD_extract_string_arg(const char *str, uint8_t arg_index, char *result, uint16_t result_max_len);

uint8_t TCMD_ascii_to_double(const char *str, uint32_t str_len, double *result);
uint8_t TCMD_extract_double_arg(const char *str, uint32_t str_len, uint8_t arg_index, double *result);

#endif // __INCLUDE_GUARD__TELECOMMAND_ARGS_HELPERS_H__
