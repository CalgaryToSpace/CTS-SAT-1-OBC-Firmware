#include "main.h"
#include "stdio.h"
#include <stdint.h>

#include "temperature_sensor/temperature_sensor.h"

const uint16_t TEMP_SENSOR_device_addr =  0x91;
const uint16_t TEMP_SENSOR_temp_register_addr  = 0x00;

// write to this register to change the precision
const uint16_t TEMP_SENSOR_config_register_addr = 0x01;

// 0x00 for nine bit, 0x01 for 10 bit, 0x02 for 11 bit, 0x03 for 12 bit
const uint8_t TEMP_SENSOR_precision = 0x00;


/// @brief Reads the temperature from the STDS75DS2F and stores it in the provided variable pointer result. 
///         Precision can be configured from 1 to 4 bits.
///         Temperature range is -55 to 125 degrees celsius with +/- 3 degrees celsius over the whole range.
/// @param result
/// - Arg 0: Memory location to store the temperature result.
/// @return 0 if successful, 1 if write error, 2 if read error.
uint8_t read_temperature(int32_t *result)
{
    HAL_StatusTypeDef status;
    uint8_t buf[2];
    uint8_t precision = TEMP_SENSOR_precision;

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

    // convert the raw temperature bytes to degrees celsius
    *result = TEMP_SENSOR_convert_raw_to_deg_c(buf, TEMP_SENSOR_nine_bit_precision_coefficient, TEMP_SENSOR_NINE_BIT_PRECISION_INSIGNIFICANT_BYTES);


    return 0;
}

/// @brief Converts the raw bytes provided to a temperature value in celsius * 10000 based on the provided precision.
///        Works on the four MSBs. Reference https://www.st.com/resource/en/datasheet/stds75.pdf page 15 and 18 for 
///        information on calculations and format of temperature data.
/// @param raw_bytes
/// - Arg 0: Bytes to convert to celsius float value
/// @param TEMP_SENSOR_data_precision_coefficient
/// - Arg 1: Coefficient used to get the temperature in the correct precision.
/// @param TEMP_SENSOR_data_precision_insignificant_bits
/// - Arg 2: Value used to remove always zero bits and bits not used the provided precision.
/// @return the temperature in celsius after conversion.
int32_t TEMP_SENSOR_convert_raw_to_deg_c(uint8_t raw_bytes[], uint8_t TEMP_SENSOR_data_precision_coefficient, 
                                Temperature_Sensor_Data_Precision_Insignificant_Bytes_t TEMP_SENSOR_data_precision_insignificant_bits)
{

    int16_t val = raw_bytes[0] << 8 | raw_bytes[1];

    // remove the always zero bytes and the bytes that are not used for our precision
    val = val >> TEMP_SENSOR_data_precision_insignificant_bits;

    // calculate the temperature in degrees celsius
    float temp = (float)val / TEMP_SENSOR_data_precision_coefficient;

    // multiply by 10000 to get rid of the fractional portions (C * 10000)
    int32_t result = temp * 10000;

    return result;
}