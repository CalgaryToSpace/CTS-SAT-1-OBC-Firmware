#include "temperature_sensor/temperature_sensor.h"
#include "unit_tests/unit_test_helpers.h"


uint8_t TEST_EXEC__TEMP_calculate_decimal_portion(void)
{
    // one bit precision tests
    TEST_ASSERT(0.5 == calculate_decimal_portion(0x80, TEMP_SENSOR_ONE_BIT_PRECISION));
    TEST_ASSERT(0 == calculate_decimal_portion(0x00, TEMP_SENSOR_ONE_BIT_PRECISION));
    TEST_ASSERT(0.5 == calculate_decimal_portion(0xA0, TEMP_SENSOR_ONE_BIT_PRECISION));

    // two bit precision tests
    TEST_ASSERT(0.25 == calculate_decimal_portion(0x40, TEMP_SENSOR_TWO_BIT_PRECISION));
    TEST_ASSERT(0 == calculate_decimal_portion(0x00, TEMP_SENSOR_TWO_BIT_PRECISION));
    TEST_ASSERT(0.5 == calculate_decimal_portion(0x80, TEMP_SENSOR_TWO_BIT_PRECISION));

    // three bit precision tests
    TEST_ASSERT(0.125 == calculate_decimal_portion(0x20, TEMP_SENSOR_THREE_BIT_PRECISION));
    TEST_ASSERT(0 == calculate_decimal_portion(0x00, TEMP_SENSOR_THREE_BIT_PRECISION));
    TEST_ASSERT(0.5 == calculate_decimal_portion(0x80, TEMP_SENSOR_THREE_BIT_PRECISION));


    // four bit precision tests
    TEST_ASSERT(0.0625 == calculate_decimal_portion(0x10, TEMP_SENSOR_FOUR_BIT_PRECISION));
    TEST_ASSERT(0 == calculate_decimal_portion(0x00, TEMP_SENSOR_FOUR_BIT_PRECISION));
    TEST_ASSERT(0.1875 == calculate_decimal_portion(0x30, TEMP_SENSOR_FOUR_BIT_PRECISION));

    // An always zero bit is set for some reason (these bits are ignored)
    TEST_ASSERT(0 == calculate_decimal_portion(0x01, TEMP_SENSOR_ONE_BIT_PRECISION));
    TEST_ASSERT(0 == calculate_decimal_portion(0x01, TEMP_SENSOR_TWO_BIT_PRECISION));
    TEST_ASSERT(0 == calculate_decimal_portion(0x01, TEMP_SENSOR_THREE_BIT_PRECISION));
    TEST_ASSERT(0 == calculate_decimal_portion(0x01, TEMP_SENSOR_FOUR_BIT_PRECISION));

    return 0;
}