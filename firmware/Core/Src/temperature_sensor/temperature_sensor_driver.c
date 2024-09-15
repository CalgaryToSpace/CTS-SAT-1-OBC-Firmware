#include "main.h"
#include <stdint.h>
#include "temperature_sensor.h"
#include "stdio.h"

const uint16_t device_addr =  0x91;
const temp_register_addr  = 0x00;
const config_register_addr = 0x01;
const DataPrecision dataPrecision = ONE_BIT_PRECISION;


/// @brief Reads the temperature from the STDS75DS2F and stores it in the provided variable pointer result. 
///         Precision can be configured from 1 to 4 bits.
///         Temperature range is -55 to 125 degrees celsius with +/- 3 degrees celsius over the whole range.
/// @param result
/// - Arg 0: Memory location to store the temperature result.
/// @return 0 if successful, 1 if write error, 2 if read error.
int read_temperature(float *result)
{
    HAL_StatusTypeDef status;
    uint8_t buf[2];
    uint8_t precision = dataPrecision;

    // write the precision value to the config register
    status = HAL_I2C_Mem_Write(&hi2c1, device_addr << 1, config_register_addr, 1, &precision, 1, HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
        // write error
        return 1;
    }

    // read the temperature from the temp register
    status = HAL_I2C_Mem_Read(&hi2c1, device_addr << 1, temp_register_addr, 1, buf, 2, HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
        // read error.
        return 2;
    }

    // calculate the value to the right of the decimal.
    float decimalPortion = calculate_decimal_portion(buf[1], dataPrecision);

    // add the integer portion with the decimal portion and store it.
    *result = (float)((int8_t)buf[0]) + decimalPortion;

    return 0;
}

/// @brief converts the bits provided to a decimal value based on the provided precision. Works on the four MSBs.
/// @param bits
/// - Arg 0: Bits to convert to decimal value
/// @param dataPrecision
/// - Arg 1: Precision to which we want the output decimal value
/// @return the decimal after conversion.
float calculate_decimal_portion(uint8_t bits, DataPrecision dataPrecision)
{
    float decimal = 0.0f;
    uint8_t maskedBits = 0;

    switch (dataPrecision)
    {
        case ONE_BIT_PRECISION: 
            maskedBits = bits & 0x80;
            maskedBits = maskedBits >> 7;
            decimal = (float)maskedBits / 2;
            break;

        case TWO_BIT_PRECISION:
            maskedBits = bits & 0xC0;
            maskedBits = maskedBits >> 6;
            decimal = (float)maskedBits / 4;
            break;
        
        case THREE_BIT_PRECISION:
            maskedBits = bits & 0xE0;
            maskedBits = maskedBits >> 5;
            decimal = (float)maskedBits / 8;
            break;

        case FOUR_BIT_PRECISION:
            maskedBits = bits & 0xF0;
            maskedBits = maskedBits >> 4;
            decimal = (float)maskedBits /  16;
            break;
    }

    return decimal;
}