#include "temperature_sensor/temperature_sensor.h"
#include "unit_tests/unit_test_helpers.h"


uint8_t TEST_EXEC__TEMP_convert_raw_to_deg_c(void)
{
    uint8_t raw_data[2];

    // nine bit precision tests
    raw_data[0] = 0x00;
    raw_data[1] = 0x80;
    TEST_ASSERT(50 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_nine_bit_precision_coefficient, TEMP_SENSOR_NINE_BIT_PRECISION_INSIGNIFICANT_BYTES));
    raw_data[0] = 0x03;
    raw_data[1] = 0x80;
    TEST_ASSERT(350 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_nine_bit_precision_coefficient, TEMP_SENSOR_NINE_BIT_PRECISION_INSIGNIFICANT_BYTES));
    raw_data[0] = 0x9C;
    raw_data[1] = 0x80;
    TEST_ASSERT(-9950 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_nine_bit_precision_coefficient, TEMP_SENSOR_NINE_BIT_PRECISION_INSIGNIFICANT_BYTES));

    // ten bit precision
    raw_data[0] = 0x14;
    raw_data[1] = 0xC0;
    TEST_ASSERT(2075 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_ten_bit_precision_coefficient, TEMP_SENSOR_TEN_BIT_PRECISION_INSIGNIFICANT_BYTES));
    raw_data[0] = 0x00;
    raw_data[1] = 0x80;
    TEST_ASSERT(50 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_ten_bit_precision_coefficient, TEMP_SENSOR_TEN_BIT_PRECISION_INSIGNIFICANT_BYTES));
    raw_data[0] = 0xA0;
    raw_data[1] = 0x40;
    TEST_ASSERT(-9575 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_ten_bit_precision_coefficient, TEMP_SENSOR_TEN_BIT_PRECISION_INSIGNIFICANT_BYTES));

    // eleven bit precision
    raw_data[0] = 0x0A;
    raw_data[1] = 0x20;
    TEST_ASSERT(1012 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_eleven_bit_precision_coefficient, TEMP_SENSOR_ELEVEN_BIT_PRECISION_INSIGNIFICANT_BYTES));
    raw_data[0] = 0x00;
    raw_data[1] = 0x20;
    TEST_ASSERT(12 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_eleven_bit_precision_coefficient, TEMP_SENSOR_ELEVEN_BIT_PRECISION_INSIGNIFICANT_BYTES));
    raw_data[0] = 0xE1;
    raw_data[1] = 0xA0;
    TEST_ASSERT(-3037 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_eleven_bit_precision_coefficient, TEMP_SENSOR_ELEVEN_BIT_PRECISION_INSIGNIFICANT_BYTES));
    
    // twelve bit precision
    raw_data[0] = 0x00;
    raw_data[1] = 0x10;
    TEST_ASSERT(6 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_twelve_bit_precision_coefficient, TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES));
    raw_data[0] = 0x50;
    raw_data[1] = 0x30;
    TEST_ASSERT(8018 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_twelve_bit_precision_coefficient, TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES));
    raw_data[0] = 0xC8;
    raw_data[1] = 0x10;
    TEST_ASSERT(-5593 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_twelve_bit_precision_coefficient, TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES));
    
    // all ones
    raw_data[0] = 0xFF;
    raw_data[1] = 0xF0;
    TEST_ASSERT(-6 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_twelve_bit_precision_coefficient, TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES));

    // max negative number
    raw_data[0] = 0xC9;
    raw_data[1] = 0x00;
    TEST_ASSERT(-5500 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_eleven_bit_precision_coefficient, TEMP_SENSOR_ELEVEN_BIT_PRECISION_INSIGNIFICANT_BYTES));

    // max positive number measurable
    raw_data[0] = 0x7D;
    raw_data[1] = 0x00;
    TEST_ASSERT(12500 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_twelve_bit_precision_coefficient, TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES));
    
    // An always zero bit is set for some reason (these bits are ignored)
    raw_data[0] = 0x00;
    raw_data[1] = 0x0F;
    TEST_ASSERT(0 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_twelve_bit_precision_coefficient, TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES));

    // zero tests
    raw_data[0] = 0x00;
    raw_data[1] = 0x00;
    TEST_ASSERT(0 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_nine_bit_precision_coefficient, TEMP_SENSOR_NINE_BIT_PRECISION_INSIGNIFICANT_BYTES));
    TEST_ASSERT(0 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_ten_bit_precision_coefficient, TEMP_SENSOR_TEN_BIT_PRECISION_INSIGNIFICANT_BYTES));
    TEST_ASSERT(0 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_eleven_bit_precision_coefficient, TEMP_SENSOR_ELEVEN_BIT_PRECISION_INSIGNIFICANT_BYTES));
    TEST_ASSERT(0 == TEMP_SENSOR_convert_raw_to_deg_c(raw_data, TEMP_SENSOR_twelve_bit_precision_coefficient, TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES));

    return 0;
}