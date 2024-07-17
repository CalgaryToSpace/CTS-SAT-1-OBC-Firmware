#ifndef __INCLUDE_GUARD_ANT_HELPER_H__
#define __INCLUDE_GUARD_ANT_HELPER_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

bool to_hex(char* dest, size_t dest_len, const uint8_t* values, size_t val_len);

#endif /* __INCLUDE_GUARD_ANT_HELPER_H__ */