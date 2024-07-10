
#ifndef __INCLUDE_GUARD__UNIT_TEST_HELPERS_H__
#define __INCLUDE_GUARD__UNIT_TEST_HELPERS_H__

#include "main.h"

#define TEST_ASSERT(x) if (!(x)) { return 1; }
#define TEST_ASSERT_TRUE(x) if (!(x)) { return 1; }
#define TEST_ASSERT_FALSE(x) if ((x)) { return 1; }

uint8_t TEST_EXEC__TCMD_hex_string_to_byte_array();


#endif // __INCLUDE_GUARD__UNIT_TEST_HELPERS_H__
