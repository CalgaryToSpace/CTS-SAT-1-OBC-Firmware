#include "unit_tests/unit_test_helpers.h"
#include "transforms/sha256.h"
#include <string.h>
#include "debug_tools/debug_uart.h"
#include <stdint.h>
uint8_t TEST_EXEC__compute_sha256_hash() {
    const uint8_t message[] = "abc" ;
    const uint8_t expected[] = {0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 
                                0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23, 
                                0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 
                                0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};
    uint8_t digest[32];
    compute_sha256_hash(message, 3, digest);
    TEST_ASSERT_TRUE(memcmp(digest, expected, 32) == 0);
    return 0;
}