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

    if (success_result == 0)
        snprintf(response_output_buf, 35, "Temperature(C * 10000): %ld\n", temperature);
    else
        snprintf(response_output_buf, 35, "Temperature read fail!\n");

    return 0;
}