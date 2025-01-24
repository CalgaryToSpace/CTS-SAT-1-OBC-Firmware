
#ifndef INCLUDE_GUARD__TEST_COMMAND_PARSER_H__
#define INCLUDE_GUARD__TEST_COMMAND_PARSER_H__

#include "main.h"


uint8_t TEST_EXEC__TCMD_is_char_alphanumeric();

uint8_t TEST_EXEC__TCMD_check_starts_with_device_id();
uint8_t TEST_EXEC__TCMD_get_suffix_tag_uint64();

uint8_t TEST_EXEC__TCMD_ascii_to_double();

uint8_t TEST_EXEC_TCMD_parse_full_telecommand();

#endif // INCLUDE_GUARD__TEST_COMMAND_PARSER_H__
