#ifndef INCLUDE_GUARD__LFS_SEARCH_TELECOMMAND_DEFS
#define INCLUDE_GUARD__LFS_SEARCH_TELECOMMAND_DEFS


#include <stdint.h>

uint8_t TCMDEXEC_fs_count_str_occurrences(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_fs_find_nth_str_occurrence(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_fs_count_hex_occurrences(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_fs_find_nth_hex_occurrence(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
);


#endif // INCLUDE_GUARD__LFS_SEARCH_TELECOMMAND_DEFS
