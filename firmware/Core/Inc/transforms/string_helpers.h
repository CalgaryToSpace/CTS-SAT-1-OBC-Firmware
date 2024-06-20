#ifndef STRING_HELPERS_H
#define STRING_HELPERS_H
#include <stdint.h>
// uint8_t split_string_by_delimiter(const char *input_str, const uint8_t input_str_len, const char delim, char result[][20], const uint8_t max_segments);

uint8_t split_string_by_delimiter(char **input_str, const uint8_t input_str_len, const char delim, char *result);
#endif // STRING_HELPERS_H