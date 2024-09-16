#include "temperature_sensor/temperature_sensor.h"
#include "unit_tests/unit_test_helpers.h"


uint8_t TEST_EXEC__TEMP_calculate_decimal_portion(void)
{
    // one bit precision tests
    TEST_ASSERT(0.5 == calculate_decimal_portion(0x80, ONE_BIT_PRECISION));
    TEST_ASSERT(0 == calculate_decimal_portion(0x00, ONE_BIT_PRECISION));
    TEST_ASSERT(0.5 == calculate_decimal_portion(0xA0, ONE_BIT_PRECISION));

    // two bit precision tests
    TEST_ASSERT(0.25 == calculate_decimal_portion(0x40, TWO_BIT_PRECISION));
    TEST_ASSERT(0 == calculate_decimal_portion(0x00, TWO_BIT_PRECISION));
    TEST_ASSERT(0.5 == calculate_decimal_portion(0x80, TWO_BIT_PRECISION));

    // three bit precision tests
    TEST_ASSERT(0.125 == calculate_decimal_portion(0x20, THREE_BIT_PRECISION));
    TEST_ASSERT(0 == calculate_decimal_portion(0x00, THREE_BIT_PRECISION));
    TEST_ASSERT(0.5 == calculate_decimal_portion(0x80, THREE_BIT_PRECISION));


    // four bit precision tests
    TEST_ASSERT(0.0625 == calculate_decimal_portion(0x10, FOUR_BIT_PRECISION));
    TEST_ASSERT(0 == calculate_decimal_portion(0x00, FOUR_BIT_PRECISION));
    TEST_ASSERT(0.1875 == calculate_decimal_portion(0x30, FOUR_BIT_PRECISION));

    // An always zero bit is set for some reason (these bits are ignored)
    TEST_ASSERT(0 == calculate_decimal_portion(0x01, ONE_BIT_PRECISION));
    TEST_ASSERT(0 == calculate_decimal_portion(0x01, TWO_BIT_PRECISION));
    TEST_ASSERT(0 == calculate_decimal_portion(0x01, THREE_BIT_PRECISION));
    TEST_ASSERT(0 == calculate_decimal_portion(0x01, FOUR_BIT_PRECISION));

    return 0;
}