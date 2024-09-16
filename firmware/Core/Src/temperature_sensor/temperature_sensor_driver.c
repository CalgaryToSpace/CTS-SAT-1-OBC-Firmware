#include "main.h"
#include "stdio.h"
#include <stdint.h>

#include "temperature_sensor/temperature_sensor.h"

const uint16_t TEMP_SENSOR_device_addr =  0x91;
const uint16_t TEMP_SENSOR_temp_register_addr  = 0x00;
const uint16_t TEMP_SENSOR_config_register_addr = 0x01;
const Temperature_Sensor_Data_Precision_t TEMP_SENSOR_data_precision = TEMP_SENSOR_ONE_BIT_PRECISION;


/// @brief Reads the temperature from the STDS75DS2F and stores it in the provided variable pointer result. 
///         Precision can be configured from 1 to 4 bits.
///         Temperature range is -55 to 125 degrees celsius with +/- 3 degrees celsius over the whole range.
/// @param result
/// - Arg 0: Memory location to store the temperature result.
/// @return 0 if successful, 1 if write error, 2 if read error.
uint8_t read_temperature(float *result)
{
    HAL_StatusTypeDef status;
    uint8_t buf[2];
    uint8_t precision = TEMP_SENSOR_data_precision;

    // write the precision value to the config register
    status = HAL_I2C_Mem_Write(&hi2c1, TEMP_SENSOR_device_addr << 1, TEMP_SENSOR_config_register_addr, 1, &precision, 1, HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
        // write error
        return 1;
    }

    // read the temperature from the temp register
    status = HAL_I2C_Mem_Read(&hi2c1, TEMP_SENSOR_device_addr << 1, TEMP_SENSOR_temp_register_addr, 1, buf, 2, HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
        // read error.
        return 2;
    }

    // calculate the value to the right of the decimal.
    float decimalPortion = calculate_decimal_portion(buf[1], TEMP_SENSOR_data_precision);

    // add the integer portion with the decimal portion and store it.
    *result = (float)((int8_t)buf[0]) + decimalPortion;

    return 0;
}

/// @brief Converts the bits provided to a decimal value based on the provided precision. Works on the four MSBs.
///        Reference https://www.st.com/resource/en/datasheet/stds75.pdf page 15 and 18 for information on calculations and
///        format of temperature data.
/// @param bits
/// - Arg 0: Bits to convert to decimal value
/// @param TEMP_SENSOR_data_precision
/// - Arg 1: Precision to which we want the output decimal value
/// @return the decimal after conversion.
float calculate_decimal_portion(uint8_t bits, Temperature_Sensor_Data_Precision_t TEMP_SENSOR_data_precision)
{
    float decimal = 0.0f;
    uint8_t masked_bits = 0;

    switch (TEMP_SENSOR_data_precision)
    {
        case TEMP_SENSOR_ONE_BIT_PRECISION: 
            masked_bits = bits & 0x80; // 0b100000000
            masked_bits = masked_bits >> 7;
            decimal = (float)masked_bits / 2;
            break;

        case TEMP_SENSOR_TWO_BIT_PRECISION:
            masked_bits = bits & 0xC0; // 0b11000000
            masked_bits = masked_bits >> 6;
            decimal = (float)masked_bits / 4;
            break;
        
        case TEMP_SENSOR_THREE_BIT_PRECISION:
            masked_bits = bits & 0xE0; // 0b11100000
            masked_bits = masked_bits >> 5;
            decimal = (float)masked_bits / 8;
            break;

        case TEMP_SENSOR_FOUR_BIT_PRECISION:
            masked_bits = bits & 0xF0; // 0b11110000
            masked_bits = masked_bits >> 4;
            decimal = (float)masked_bits /  16;
            break;
    }

    return decimal;
}