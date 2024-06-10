#ifndef HELPERS_H
#define HELPERS_H

#include <stdint.h>

uint8_t split_string_by_delimiter(const uint8_t *input_str, const uint8_t input_str_len, const uint8_t delim, uint8_t result[][20], const uint8_t max_segments);

#endif