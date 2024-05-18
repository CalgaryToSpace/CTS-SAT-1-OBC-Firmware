#include "unit_tests/unit_test_helpers.h"
#include "unit_tests/test_tests.h"

uint8_t TEST_EXEC__test_asserts() {
    TEST_ASSERT(1 == 1);
    TEST_ASSERT(1 == 1);

    TEST_ASSERT_TRUE(8 == 8);
    TEST_ASSERT_FALSE(8 == 7);

    TEST_ASSERT_FALSE(8 == 64);

    return 0;
}

uint8_t TEST_EXEC__this_test_always_fails() {
    TEST_ASSERT(1 == 0);
    return 0;
}
