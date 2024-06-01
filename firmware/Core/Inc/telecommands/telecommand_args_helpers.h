#ifndef __INCLUDE_GUARD__TELECOMMAND_ARGS_HELPERS_H__
#define __INCLUDE_GUARD__TELECOMMAND_ARGS_HELPERS_H__

#include <stdint.h>

#define TCMD_MAX_STRING_LEN 512

uint8_t TCMD_ascii_to_uint64(const char *str, uint32_t str_len, uint64_t *result);
uint8_t TCMD_extract_uint64_arg(const char *str, uint32_t str_len, uint8_t arg_index, uint64_t *result);
uint8_t TCMD_get_arg_info(const char *str, uint32_t str_len, uint8_t arg_index, uint32_t *first_index, uint32_t *last_index, uint32_t *arg_length);
uint8_t TCMD_arg_as_string(const char *str, uint32_t str_len, uint8_t arg_index, char *result);

#endif // __INCLUDE_GUARD__TELECOMMAND_ARGS_HELPERS_H__
