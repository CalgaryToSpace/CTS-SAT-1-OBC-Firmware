#include "telecommands/obc_systems_telecommand_defs.h"
#include "obc_systems/obc_temperature_sensor.h"
#include "obc_systems/adc_vbat_monitor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "main.h"

/// @brief  Reads the temperature from the STDS75DS2F and stores it in the provided variable temperature.
///         Temperature range is -55 to 125 degrees celsius with +/- 3 degrees celsius accuracy over the whole range.
/// @param args_str
/// - Arg 0: Precision we want the temperature to be (9-12 bits).
/// @return 0 if successful, 1 if error.
/// @note There are better ways to get the temperature.
uint8_t TCMDEXEC_obc_read_temperature_complex(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    int32_t temperature;
    uint32_t temp_precision_conversion_delay_ms;
    uint32_t temp_scaling_factor;

    // Convert user requested precision.
    const uint8_t temp_precision = atoi(args_str);

    const uint8_t status = OBC_TEMP_SENSOR_set_temp_precision(
        temp_precision, &temp_precision_conversion_delay_ms, &temp_scaling_factor
    );
    switch (status)
    {
        case 0:
            break;
        case 1:
        {
            snprintf(response_output_buf, response_output_buf_len, "Error writing config register.");
            return 1;
        }
        case 2:
        {
            snprintf(
                response_output_buf, response_output_buf_len,
                "Invalid temperature precision provided. Arg must be in range 9-12."
            );
            return 2;
        }
    }

    // Give time for sensor to convert value.
    HAL_Delay(temp_precision_conversion_delay_ms);

    // Read temperature.
    uint8_t success_result = OBC_TEMP_SENSOR_read_temperature(&temperature);

    // Handle result.
    if (success_result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Temperature read fail!");
        return 3;
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "Temperature(C * %lu): %ld",
        temp_scaling_factor,
        temperature
    );

    return 0;
}

/// @brief Reads the temperature from the STDS75DS2F in centi-Celsius.
/// @return 0 if successful, >0 if error.
/// @note Temperature range is -55 to 125 degrees celsius with +/- 3 degrees celsius accuracy over the whole range.
uint8_t TCMDEXEC_obc_read_temperature(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const int32_t temp_cC = OBC_TEMP_SENSOR_get_temperature_cC();
    if (temp_cC == OBC_TEMP_SENSOR_ERROR_TEMPERATURE_CC) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Temperature read fail! Temperature: %" PRIi32 " cC",
            temp_cC
        );
        return 1;
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "OBC Temperature: %" PRIi32 " cC",
        temp_cC
    );

    return 0;
}

/// @brief Read the battery voltage using the ADC. Returns the result in mV. Voltage divider ratio is already applied.
/// @param args_str No arguments.
/// @return 
uint8_t TCMDEXEC_obc_adc_read_vbat_voltage(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    // Read the voltage from the ADC.
    const int16_t vbat_mV = OBC_read_vbat_with_adc_mV();

    if (vbat_mV == -9999) {
        snprintf(response_output_buf, response_output_buf_len, "VBAT ADC read error.");
        return 1;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "{\"vbat_voltage_mV\":%d}", vbat_mV);
    return 0;
}
