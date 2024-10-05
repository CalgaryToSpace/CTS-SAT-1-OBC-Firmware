#include "telecommands/temperature_sensor_telecommand_defs.h"
#include "temperature_sensor/temperature_sensor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"


/// @brief  Reads the temperature from the STDS75DS2F and stores it in the provided variable temperature. 
///         Temperature range is -55 to 125 degrees celsius with +/- 3 degrees celsius accuracy over the whole range.
/// @param args_str
/// - None
/// @return 0 if successful, 1 if error.
uint8_t TCMDEXEC_read_temperature(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, 
                                char *response_output_buf, uint16_t response_output_buf_len)
{

    int32_t temperature;

    uint8_t success_result = read_temperature(&temperature);

    uint8_t precision = TEMP_SENSOR__get_temp_precision();

    switch (precision) {
        case 0x00: precision = 9; break;
        case 0x01: precision = 10; break;
        case 0x02: precision = 11; break;
        case 0x03: precision = 12; break;
        default: precision = 0; break; // Handle unexpected cases
    }

    if (success_result == 0)
        snprintf(response_output_buf, response_output_buf_len, "Temperature(%d bit): %ld\n", precision, temperature);
    else
        snprintf(response_output_buf, response_output_buf_len, "Temperature read fail!\n");

    return 0;
}


/// @brief  Set the temperature sensor temperature precision value in the config register.
/// @param args_str
///  Arg 0: Precision we want the temperature to be (9-12 bits).
/// @return 0 if successful, 1 if error.
uint8_t TCMDEXEC_set_temperature_precision(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, 
                                char *response_output_buf, uint16_t response_output_buf_len)
{
    const uint8_t temp_precision = atoi(args_str);

    uint8_t status = TEMP_SENSOR__set_temp_precision(temp_precision, 1);

    if (!status)
    {
        snprintf(response_output_buf, response_output_buf_len, "Temperature precision set to %d bit precision.\n", temp_precision);
    }
    else if (status == 2)
    {
        snprintf(response_output_buf, response_output_buf_len, "Invalid temperature precision provided. Arg must be in range 9-12.\n");
    }
    else
    {
        snprintf(response_output_buf, response_output_buf_len, "Unable to set temperature precision.\n");
    }
    
    return 0;
}

