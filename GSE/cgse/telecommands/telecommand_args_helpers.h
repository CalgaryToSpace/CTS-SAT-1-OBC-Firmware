#ifndef __INCLUDE_GUARD__TELECOMMAND_ARGS_HELPERS_H__
#define __INCLUDE_GUARD__TELECOMMAND_ARGS_HELPERS_H__

#include <stdint.h>

static inline uint8_t TCMD_ascii_to_uint64(const char *str, uint32_t str_len, uint64_t *result) {return 0;}
static inline uint8_t TCMD_extract_uint64_arg(const char *str, uint32_t str_len, uint8_t arg_index, uint64_t *result) {return 0;}

#endif // __INCLUDE_GUARD__TELECOMMAND_ARGS_HELPERS_H__
