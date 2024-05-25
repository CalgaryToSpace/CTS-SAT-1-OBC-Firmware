
#ifndef __INCLUDE_GUARD__TELECOMMAND_PARSER_H__
#define __INCLUDE_GUARD__TELECOMMAND_PARSER_H__

#include "telecommands/telecommand_definitions.h"


#define TCMD_PREFIX_STR "CTS1+"
#define TCMD_PREFIX_STR_LEN 5


uint8_t TCMD_is_char_alphanumeric(char c);
uint8_t TCMD_is_char_valid_telecommand_name_char(char c);

uint8_t TCMD_check_starts_with_device_id(const char *tcmd_str, uint32_t tcmd_str_len);
int32_t TCMD_parse_telecommand_get_index(const char *tcmd_str, uint32_t tcmd_str_len);

#endif // __INCLUDE_GUARD__TELECOMMAND_PARSER_H__

