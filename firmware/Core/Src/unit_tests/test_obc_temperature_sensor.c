#include "stddef.h"

#include "obc_systems/obc_temperature_sensor.h"
#include "unit_tests/unit_test_helpers.h"


uint8_t TEST_EXEC__OBC_TEMP_SENSOR_convert_raw_to_deg_c(void)
{
    uint8_t raw_data[2];

    // nine bit precision tests
    raw_data[0] = 0x00;
    raw_data[1] = 0x80;
    TEST_ASSERT(5 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_nine_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_NINE_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_nine_bit_scaling_factor));
    raw_data[0] = 0x03;
    raw_data[1] = 0x80;
    TEST_ASSERT(35 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_nine_bit_precision_coefficient,
                                    OBC_TEMP_SENSOR_NINE_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_nine_bit_scaling_factor));
    raw_data[0] = 0x9C;
    raw_data[1] = 0x80;
    TEST_ASSERT(-995 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_nine_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_NINE_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_nine_bit_scaling_factor));

    // ten bit precision
    raw_data[0] = 0x14;
    raw_data[1] = 0xC0;
    TEST_ASSERT(2075 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_ten_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_TEN_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_ten_bit_scaling_factor));
    raw_data[0] = 0x00;
    raw_data[1] = 0x80;
    TEST_ASSERT(50 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_ten_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_TEN_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_ten_bit_scaling_factor));
    raw_data[0] = 0xA0;
    raw_data[1] = 0x40;
    TEST_ASSERT(-9575 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_ten_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_TEN_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_ten_bit_scaling_factor));

    // eleven bit precision
    //10.125
    raw_data[0] = 0x0A;
    raw_data[1] = 0x20;
    TEST_ASSERT(10125 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_eleven_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_ELEVEN_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_eleven_bit_scaling_factor));
    // 0.125
    raw_data[0] = 0x00;
    raw_data[1] = 0x20;
    TEST_ASSERT(125 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_eleven_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_ELEVEN_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_eleven_bit_scaling_factor));
    // 30.375
    raw_data[0] = 0xE1;
    raw_data[1] = 0xA0;
    TEST_ASSERT(-30375 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_eleven_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_ELEVEN_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_eleven_bit_scaling_factor));
    
    // twelve bit precision
    // 0.0625
    raw_data[0] = 0x00;
    raw_data[1] = 0x10;
    TEST_ASSERT(625 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_twelve_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_twelve_bit_scaling_factor));
    // 80.1875
    raw_data[0] = 0x50;
    raw_data[1] = 0x30;
    TEST_ASSERT(801875 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_twelve_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_twelve_bit_scaling_factor));
    // 55.9375
    raw_data[0] = 0xC8;
    raw_data[1] = 0x10;
    TEST_ASSERT(-559375 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_twelve_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_twelve_bit_scaling_factor));
    
    // all ones
    raw_data[0] = 0xFF;
    raw_data[1] = 0xF0;
    TEST_ASSERT(-625 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_twelve_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_twelve_bit_scaling_factor));

    // max negative number
    raw_data[0] = 0xC9;
    raw_data[1] = 0x00;
    TEST_ASSERT(-55000 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_eleven_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_ELEVEN_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_eleven_bit_scaling_factor));

    // max positive number measurable
    raw_data[0] = 0x7D;
    raw_data[1] = 0x00;
    TEST_ASSERT(1250000 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_twelve_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_twelve_bit_scaling_factor));
    
    // An always zero bit is set for some reason (these bits are ignored)
    raw_data[0] = 0x00;
    raw_data[1] = 0x0F;
    TEST_ASSERT(0 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_twelve_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_twelve_bit_scaling_factor));

    // zero tests
    raw_data[0] = 0x00;
    raw_data[1] = 0x00;
    TEST_ASSERT(0 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_nine_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_NINE_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_nine_bit_scaling_factor));
    TEST_ASSERT(0 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_ten_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_TEN_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_ten_bit_scaling_factor));
    TEST_ASSERT(0 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_eleven_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_ELEVEN_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_eleven_bit_scaling_factor));
    TEST_ASSERT(0 == OBC_TEMP_SENSOR_convert_raw_to_deg_c(raw_data, OBC_TEMP_SENSOR_twelve_bit_precision_coefficient, 
                                    OBC_TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES, OBC_TEMP_SENSOR_twelve_bit_scaling_factor));

    return 0;
}

uint8_t TEST_EXEC__OBC_TEMP_SENSOR_configure_precision_values(void)
{   
    struct Set_Precision_Data precision_data;

    // user sets the precision to 9 bit
    TEST_ASSERT(0 == OBC_TEMP_SENSOR_configure_precision_values(9, &precision_data));
    TEST_ASSERT(150 == precision_data.conversion_delay_ms);
    TEST_ASSERT(OBC_TEMP_SENSOR_nine_bit_precision_coefficient == precision_data.precision_coefficient);
    TEST_ASSERT(OBC_TEMP_SENSOR_NINE_BIT_PRECISION_INSIGNIFICANT_BYTES == precision_data.precision_insignificant_bits);
    TEST_ASSERT(10 == precision_data.precision_scaling_factor);
    TEST_ASSERT(0x00 == precision_data.config_write_data);

    // user sets the precision to 10 bit
    TEST_ASSERT(0 == OBC_TEMP_SENSOR_configure_precision_values(10, &precision_data));
    TEST_ASSERT(300 == precision_data.conversion_delay_ms);
    TEST_ASSERT(OBC_TEMP_SENSOR_ten_bit_precision_coefficient == precision_data.precision_coefficient);
    TEST_ASSERT(OBC_TEMP_SENSOR_TEN_BIT_PRECISION_INSIGNIFICANT_BYTES == precision_data.precision_insignificant_bits);
    TEST_ASSERT(100 == precision_data.precision_scaling_factor);
    TEST_ASSERT(0x20 == precision_data.config_write_data);

    // user sets the precision to 11 bit
    TEST_ASSERT(0 == OBC_TEMP_SENSOR_configure_precision_values(11, &precision_data));
    TEST_ASSERT(600 == precision_data.conversion_delay_ms);
    TEST_ASSERT(OBC_TEMP_SENSOR_eleven_bit_precision_coefficient == precision_data.precision_coefficient);
    TEST_ASSERT(OBC_TEMP_SENSOR_ELEVEN_BIT_PRECISION_INSIGNIFICANT_BYTES == precision_data.precision_insignificant_bits);
    TEST_ASSERT(1000 == precision_data.precision_scaling_factor);
    TEST_ASSERT(0x40 == precision_data.config_write_data);

    // user sets the precision to 12 bit
    TEST_ASSERT(0 == OBC_TEMP_SENSOR_configure_precision_values(12, &precision_data));
    TEST_ASSERT(1200 == precision_data.conversion_delay_ms);
    TEST_ASSERT(OBC_TEMP_SENSOR_twelve_bit_precision_coefficient == precision_data.precision_coefficient);
    TEST_ASSERT(OBC_TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES == precision_data.precision_insignificant_bits);
    TEST_ASSERT(10000 == precision_data.precision_scaling_factor);
    TEST_ASSERT(0x60 == precision_data.config_write_data);

    // user inputs an invalid precision number. Values should be the same as what they were previously set to.
    // as a user did not set the precision (it was an error).
    TEST_ASSERT(1 == OBC_TEMP_SENSOR_configure_precision_values(13, &precision_data));
    TEST_ASSERT(1200 == precision_data.conversion_delay_ms);
    TEST_ASSERT(OBC_TEMP_SENSOR_twelve_bit_precision_coefficient == precision_data.precision_coefficient);
    TEST_ASSERT(OBC_TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES == precision_data.precision_insignificant_bits);
    TEST_ASSERT(10000 == precision_data.precision_scaling_factor);
    TEST_ASSERT(0x60 == precision_data.config_write_data);

    return 0;
}
