#include "main.h"
#include "stdio.h"
#include <stdint.h>

#include "obc_temperature_sensor/obc_temperature_sensor.h"

const uint16_t OBC_TEMP_SENSOR_device_addr =  0x48;
const uint16_t OBC_TEMP_SENSOR_temp_register_addr  = 0x00;

// write to this register to change the precision
const uint16_t OBC_TEMP_SENSOR_config_register_addr = 0x01;

// 0x00 for nine bit, 0x01 for 10 bit, 0x02 for 11 bit, 0x03 for 12 bit
const uint8_t OBC_TEMP_SENSOR_precision = 0x01 << 5;

// number used to remove fractional portions of the temperature. Default to 100 for 10 bit precision.
uint16_t OBC_TEMP_SENSOR_precision_scaling_factor = 100;

// used in temp calculation.
float OBC_TEMP_SENSOR_precision_coefficient = OBC_TEMP_SENSOR_ten_bit_precision_coefficient;

Temperature_Sensor_Data_Precision_Insignificant_Bytes_t OBC_TEMP_SENSOR_precision_insignificant_bits = OBC_TEMP_SENSOR_TEN_BIT_PRECISION_INSIGNIFICANT_BYTES;


/// @brief Reads the temperature from the STDS75DS2F and stores it in the provided variable pointer result. 
///         Precision can be configured from 9 to 12 bits.
///         Temperature range is -55 to 125 degrees celsius with +/- 3 degrees celsius over the whole range.
/// @param result
/// - Arg 0: Memory location to store the temperature result.
/// @return 0 if successful, 1 if config register error, 2 if temp register error.
uint8_t OBC_TEMP_SENSOR__read_temperature(int32_t *result)
{
    HAL_StatusTypeDef status;
    uint8_t buf[2];

    // read the temperature from the temp register
    status = HAL_I2C_Mem_Read(&hi2c4, OBC_TEMP_SENSOR_device_addr << 1, OBC_TEMP_SENSOR_temp_register_addr, 1, buf, 2, HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
        // read error.
        return 2;
    }

    // convert the raw temperature bytes to degrees celsius
    *result = OBC_TEMP_SENSOR__convert_raw_to_deg_c(buf, OBC_TEMP_SENSOR_precision_coefficient, OBC_TEMP_SENSOR_precision_insignificant_bits, 
                                            OBC_TEMP_SENSOR_precision_scaling_factor);

    return 0;
}


/// @brief Converts the raw bytes provided to a temperature in celsius and multiplied by a scaling factor based on the provided precision.
///        Works on the four MSBs. Reference https://www.st.com/resource/en/datasheet/stds75.pdf page 15 and 18 for 
///        information on calculations and format of temperature data.
/// @param raw_bytes
/// - Arg 0: Bytes to convert to celsius float value
/// @param TEMP_SENSOR_data_precision_coefficient
/// - Arg 1: Coefficient used to get the temperature in the correct precision.
/// @param TEMP_SENSOR_data_precision_insignificant_bits
/// - Arg 2: Value used to remove always zero bits and bits not used the provided precision.
/// @param precision_scaling_factor
/// - Arg 3: value used to convert the temperature into a representation with no fractional parts.
/// @return the temperature in celsius after conversion.
int32_t OBC_TEMP_SENSOR__convert_raw_to_deg_c(uint8_t raw_bytes[], uint8_t precision_coefficient, 
                                Temperature_Sensor_Data_Precision_Insignificant_Bytes_t precision_insignificant_bits, 
                                uint16_t precision_scaling_factor)
{

    int16_t val = raw_bytes[0] << 8 | raw_bytes[1];

    // remove the always zero bytes and the bytes that are not used for our precision
    val = val >> precision_insignificant_bits;

    // calculate the temperature in degrees celsius
    float temp = (float)val / precision_coefficient;

    // multiply by TEMP_SENSOR_custom_precision_set to convert temp to a value without fractional bits.
    int32_t result = temp * precision_scaling_factor;

    return result;
}


/// @brief Reads the temperature precision from the config register.
///        Refer to https://www.st.com/resource/en/datasheet/stds75.pdf page 17, table 6 and 7 for more information.
/// @param None
/// @return -1 if there is an error otherwise the temperature precision in the least significant two bits as 0x01, 0x02, 0x03, 0x04.
int8_t OBC_TEMP_SENSOR__get_temp_precision()
{
    HAL_StatusTypeDef status;
    uint8_t buf;
    uint8_t precision_mask = 0x60; // 0b01100000


    status = HAL_I2C_Mem_Read(&hi2c4, OBC_TEMP_SENSOR_device_addr << 1, OBC_TEMP_SENSOR_config_register_addr, 1, &buf, 1, HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
        // return -1 if unable to read from config register
        return -1;
    }

    // return the config precision bits in the 2 least significant bits
    return (buf & precision_mask) >> 5;
}


/// @brief Set the temp temperature precision in the config register.
/// @param arg_precision
///   - Arg 0: precision to set the temperature to in decimal (9, 10, 11, 12).
/// @param temp_precision_conversion_delay_ms
///   - Arg 1: pointer to store the conversion delay. 
/// @return 0 on success, 1 on write error, 2 when passed an invalid precision value.
uint8_t OBC_TEMP_SENSOR__set_temp_precision(uint8_t arg_precision, uint32_t* temp_precision_conversion_delay_ms,
                                    uint32_t* temp_scaling_factor)
{
    HAL_StatusTypeDef status;
    struct Set_Precision_Data precision_data;

    // if a user inputs an invalid precision return 2
    if (OBC_TEMP_SENSOR__configure_precision_values(arg_precision, &precision_data))
    {
        return 2;
    }

    // update globals and conversion delay
    OBC_TEMP_SENSOR_precision_coefficient = precision_data.precision_coefficient;
    OBC_TEMP_SENSOR_precision_insignificant_bits = precision_data.precision_insignificant_bits;
    OBC_TEMP_SENSOR_precision_scaling_factor = precision_data.precision_scaling_factor;
    *temp_precision_conversion_delay_ms = precision_data.conversion_delay_ms;
    *temp_scaling_factor = precision_data.precision_scaling_factor;

    // write the precision value to the config register
    status = HAL_I2C_Mem_Write(&hi2c4, OBC_TEMP_SENSOR_device_addr << 1, OBC_TEMP_SENSOR_config_register_addr, 1, &(precision_data.config_write_data), 1, HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
        // write error
        return 1;
    }

    return 0;
}

/// @brief Configures the precision global variables in preparation for setting the precision.
/// @param arg_precision
///      precision to set the temperature to in decimal (9, 10, 11, 12).
/// @param precision_data
///     struct containing all the values needed in precision calculations.
/// @return 0 on success, 1 if user passed in an invalid precision value.
uint8_t OBC_TEMP_SENSOR__configure_precision_values(uint8_t arg_precision, struct Set_Precision_Data* precision_data)
{
    // set the scaling factor based on the inputted precision and format the precision we wish to write to the config register
    // as well as other constants used in calculations.
    switch(arg_precision)
    {
        case 9:
            precision_data->precision_scaling_factor = OBC_TEMP_SENSOR_nine_bit_scaling_factor;
            precision_data->precision_insignificant_bits = OBC_TEMP_SENSOR_NINE_BIT_PRECISION_INSIGNIFICANT_BYTES;
            precision_data->precision_coefficient = OBC_TEMP_SENSOR_nine_bit_precision_coefficient;
            precision_data->config_write_data = 0;
            precision_data->conversion_delay_ms = 150;
            break;
        case 10:
            precision_data->precision_scaling_factor = OBC_TEMP_SENSOR_ten_bit_scaling_factor;
            precision_data->precision_insignificant_bits = OBC_TEMP_SENSOR_TEN_BIT_PRECISION_INSIGNIFICANT_BYTES;
            precision_data->precision_coefficient = OBC_TEMP_SENSOR_ten_bit_precision_coefficient;
            precision_data->config_write_data = 0x01 << 5;
            precision_data->conversion_delay_ms = 300;
            break;
        case 11: 
            precision_data->precision_scaling_factor = OBC_TEMP_SENSOR_eleven_bit_scaling_factor;
            precision_data->precision_insignificant_bits = OBC_TEMP_SENSOR_ELEVEN_BIT_PRECISION_INSIGNIFICANT_BYTES;
            precision_data->precision_coefficient = OBC_TEMP_SENSOR_eleven_bit_precision_coefficient;
            precision_data->config_write_data = 0x02 << 5;
            precision_data->conversion_delay_ms = 600;
            break;
        case 12: 
            precision_data->precision_scaling_factor = OBC_TEMP_SENSOR_twelve_bit_scaling_factor;
            precision_data->precision_insignificant_bits = OBC_TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES;
            precision_data->precision_coefficient = OBC_TEMP_SENSOR_twelve_bit_precision_coefficient;
            precision_data->config_write_data = 0x03 << 5;
            precision_data->conversion_delay_ms = 1200;
            break;
        default:
            // invalid precision passed
            return 1;
    }

    return 0;
}