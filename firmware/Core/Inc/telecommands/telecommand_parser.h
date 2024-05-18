
#ifndef __INCLUDE_GUARD__TELECOMMAND_PARSER_H__
#define __INCLUDE_GUARD__TELECOMMAND_PARSER_H__

#include "telecommands/telecommand_definitions.h"


uint8_t TCMD_is_char_alphanumeric(char c);
uint8_t TCMD_is_char_valid_telecommand_name_char(char c);

int32_t TCMD_parse_telecommand_get_index(const char *tcmd_str, uint32_t tcmd_str_len);

#endif // __INCLUDE_GUARD__TELECOMMAND_PARSER_H__

