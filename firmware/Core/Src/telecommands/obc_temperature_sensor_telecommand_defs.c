#include "telecommands/temperature_sensor_telecommand_defs.h"
#include "obc_temperature_sensor/obc_temperature_sensor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

/// @brief  Reads the temperature from the STDS75DS2F and stores it in the provided variable temperature.
///         Temperature range is -55 to 125 degrees celsius with +/- 3 degrees celsius accuracy over the whole range.
/// @param args_str
/// - Arg 0: Precision we want the temperature to be (9-12 bits).
/// @return 0 if successful, 1 if error.
/// @note There are better ways to get the temperature.
uint8_t TCMDEXEC_obc_read_temperature_complex(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    int32_t temperature;
    uint8_t status;
    uint32_t temp_precision_conversion_delay_ms;
    uint32_t temp_scaling_factor;

    // convert user requested precision
    const uint8_t temp_precision = atoi(args_str);

    status = OBC_TEMP_SENSOR_set_temp_precision(temp_precision, &temp_precision_conversion_delay_ms, &temp_scaling_factor);
    switch (status)
    {
        case 0:
            break;
        case 1:
        {
            snprintf(response_output_buf, response_output_buf_len, "Error writing config register.\n");
            return 0;
        }
        case 2:
        {
            snprintf(response_output_buf, response_output_buf_len, "Invalid temperature precision provided. Arg must be in range 9-12.\n");
            return 0;
        }
    }

    // give time for sensor to convert value
    HAL_Delay(temp_precision_conversion_delay_ms);

    // Read temperature.
    uint8_t success_result = OBC_TEMP_SENSOR_read_temperature(&temperature);

    // handle result
    if (success_result == 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Temperature(C * %lu): %ld\n", temp_scaling_factor, temperature);
    }
    else
    {
        snprintf(response_output_buf, response_output_buf_len, "Temperature read fail!\n");
    }

    return 0;
}