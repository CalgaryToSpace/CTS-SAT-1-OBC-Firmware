
#include "helpers/helpers.h"

uint8_t split_string_by_delimiter(const uint8_t *str, const uint8_t str_len, const uint8_t delim, uint8_t **result)
{
    uint8_t i = 0;
    uint8_t j = 0;
    while (str[i] != '\0' && i < str_len)
    {
        if (str[i] == delim)
        {
            result[j] = &str[i + 1];
            j++;
        }
        i++;
    }
    return 0;
}