
#ifndef __INCLUDE_GUARD__TELECOMMAND_PARSER_H__
#define __INCLUDE_GUARD__TELECOMMAND_PARSER_H__

#include "telecommands/telecommand_definitions.h"


#define TCMD_PREFIX_STR "CTS1+"
#define TCMD_PREFIX_STR_LEN 5

uint8_t TCMD_is_char_alphanumeric(char c);
uint8_t TCMD_is_char_valid_telecommand_name_char(char c);

uint8_t TCMD_check_starts_with_device_id(const char *tcmd_str, uint32_t tcmd_str_len);
int32_t TCMD_parse_telecommand_get_index(const char *tcmd_str, uint32_t tcmd_str_len);

uint8_t TCMD_get_suffix_tag_uint64(const char *str, const char *tag_name, uint64_t *value_dest);

uint8_t TCMD_parse_full_telecommand(const char tcmd_str[], TCMD_TelecommandChannel_enum_t tcmd_channel,
        TCMD_parsed_tcmd_to_execute_t *parsed_tcmd_output);
uint8_t TCMD_execute_parsed_telecommand_now(const uint16_t tcmd_idx, const char args_str_no_parens[],
    TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_size
);

#endif // __INCLUDE_GUARD__TELECOMMAND_PARSER_H__

