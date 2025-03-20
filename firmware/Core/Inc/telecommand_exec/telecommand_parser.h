
#ifndef INCLUDE_GUARD__TELECOMMAND_PARSER_H__
#define INCLUDE_GUARD__TELECOMMAND_PARSER_H__

#include "telecommand_exec/telecommand_types.h"
#include "telecommand_exec/telecommand_definitions.h"

// Max len of `args_str_no_parens` in `TCMD_parsed_tcmd_to_execute_t`, including null terminator.
#define TCMD_ARGS_STR_NO_PARENS_SIZE 255

#define TCMD_PREFIX_STR "CTS1+"
#define TCMD_PREFIX_STR_LEN 5

static const uint8_t TCMD_SHA256_LENGTH_BYTES = 32;

uint8_t TCMD_is_char_alphanumeric(char c);
uint8_t TCMD_is_char_valid_telecommand_name_char(char c);

uint8_t TCMD_check_starts_with_device_id(const char *tcmd_str, uint32_t tcmd_str_len);
int32_t TCMD_parse_telecommand_get_index(const char *tcmd_str, uint32_t tcmd_str_len);

uint8_t TCMD_get_suffix_tag_uint64(const char *str, const char *tag_name, uint64_t *value_dest);
uint8_t TCMD_get_suffix_tag_hex_array(const char *str, const char *tag_name, uint8_t *value_dest);

uint8_t TCMD_parse_full_telecommand(const char tcmd_str[], TCMD_TelecommandChannel_enum_t tcmd_channel,
        TCMD_parsed_tcmd_to_execute_t *parsed_tcmd_output);

#endif // INCLUDE_GUARD__TELECOMMAND_PARSER_H__

