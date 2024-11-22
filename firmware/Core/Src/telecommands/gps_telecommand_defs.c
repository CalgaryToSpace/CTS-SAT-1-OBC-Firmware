#include "telecommands/telecommand_definitions.h"
#include "uart_handler/uart_handler.h"
#include "telecommands/eps_telecommands.h"
#include "gps/gps_internal_drivers.h"
#include "log/log.h"
#include "main.h"
#include "telecommands/telecommand_args_helpers.h"
#include "telecommands/telecommand_executor.h"
#include "debug_tools/debug_uart.h"
#include "telecommands/gps_telecommand_defs.h"
#include "gps/gps_pps_interrupt.h" 

#include "stm32l4xx_hal.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/// @brief Telecommand: Transmit a log command to the GPS receiver through UART
/// @param args_str
/// - Arg 0: Log command to be sent to GPS eg "log bestxyza once" (string)
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, > 0 error
uint8_t TCMDEXEC_gps_send_cmd_ascii(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                  char *response_output_buf, uint16_t response_output_buf_len)
{

    if (args_str == NULL)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: Empty args_str");
        return 1;
    }

    // TODO : Determine if we need to perform extra error checks on arg_str ie log command format etc

    // Adding a new line character to the log command
    char gps_log_cmd[128];
    snprintf(gps_log_cmd, sizeof(gps_log_cmd), "%s\n", args_str);
    const uint16_t gps_log_cmd_len = strlen(gps_log_cmd);

    // Allocate space to receive incoming GPS response.
    const uint16_t GPS_rx_buffer_max_size = 512;
    uint16_t GPS_rx_buffer_len = 0;
    uint8_t GPS_rx_buffer[GPS_rx_buffer_max_size];
    memset(GPS_rx_buffer, 0, GPS_rx_buffer_max_size); // Initialize all elements to 0

    // Send log command to GPS and receive response
    const uint8_t gps_cmd_response = GPS_send_cmd_get_response(
        gps_log_cmd, gps_log_cmd_len, GPS_rx_buffer, GPS_rx_buffer_len, GPS_rx_buffer_max_size
    );

    // Handle the gps_cmd_response: Perform the error checks
    // TODO: Potentially add GPS_validate_log_response function in here to validate response from the gps receiver

    if(gps_cmd_response != 0){
        LOG_message(
            LOG_SYSTEM_GPS,
            LOG_SEVERITY_NORMAL,
            LOG_SINK_ALL,
            "GPS Response Code: %d",
            gps_cmd_response
        );

    }

    snprintf(response_output_buf, response_output_buf_len, "GPS Command: '%s' successfully transmitted", args_str);

    return 0;
}

/// @brief Telecommand: Delete all agendas
/// @param args_str No arguments needed
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, 1 gps is off.
uint8_t TCMDEXEC_gps_set_enabled_pps_tracking(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

        int PIN_Status = HAL_GPIO_ReadPin(PIN_GPS_PPS_IN_GPIO_Port, PIN_GPS_PPS_IN_Pin) == GPIO_PIN_SET;
        if (PIN_Status && gps_pps_enabled == 1) {
            // Disable PPS tracking
            Disable_GPIO_EXTI();
            gps_pps_enabled = 0;
            snprintf(
                response_output_buf, response_output_buf_len,
                "PPS tracking disabled");
            return 0;
        } 
        else if (PIN_Status && gps_pps_enabled == 0) {
            // Enable PPS tracking
            GPIO_EXTI_Init();
            snprintf(
                response_output_buf, response_output_buf_len,
                "PPS tracking enabled");
            return 0;
        }
    snprintf(
                response_output_buf, response_output_buf_len,
                "Can not enable as GPS may not be active");
    return 1;
}