#include "unit_tests/unit_test_helpers.h"
#include "transforms/sha256.h"
#include <string.h>
#include "debug_tools/debug_uart.h"
#include <stdalign.h>
uint8_t TEST_EXEC__compute_sha256_hash() {
    const uint8_t message = "abc";
    const uint8_t expected[] = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    uint8_t digest[32];
    compute_sha256_hash(message, 3, digest);
    DEBUG_uart_print_str("SHA256 TEST \n");
    DEBUG_uart_print_array_hex(digest, 32);
    TEST_ASSERT_TRUE(memcmp(digest, expected, 32) == 0);
    return 0;
}