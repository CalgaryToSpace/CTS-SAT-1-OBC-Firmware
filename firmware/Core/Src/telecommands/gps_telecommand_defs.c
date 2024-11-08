#include "telecommands/telecommand_definitions.h"
#include "uart_handler/uart_handler.h"
#include "telecommands/eps_telecommands.h"
#include "gps/gps_internal_drivers.h"
#include "log/log.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


/// @brief Telecommand: Enable the interrupt mode for the GPS UART line
/// @param args_str
/// - Arg 0: Toggle status for the GPS_set_uart_interrupt_state function (integer): Should be either 0 or 1
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, > 0 error
uint8_t TCMDEXEC_gps_set_enabled(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    // Turn on power
    // TODO: Determine the appropriate channel for the GPS
    // Power requirement can be found in the OEM7 Installation Operation Manual Page 29: 3.3VDC +-5% with less than 100mV ripple
    // const char *eps_args_str = "stack_3v3,1";
    // const uint8_t eps_result = TCMDEXEC_eps_set_channel_enabled(eps_args_str, tcmd_channel, response_output_buf, response_output_buf_len);
    // if(eps_result != 0)
    // {
    //     //Error turing on the EPS
    //     return eps_result;
    // }

    // //Wait for a second
    // HAL_Delay(1000);

    // Converting the string response to an integer
    char *endptr;
    const uint8_t toggle_status = (uint8_t) strtoul(args_str, &endptr, 10);

    // Error checking for the string conversion
    if (*endptr != '\0') {
        // Invalid input
        snprintf(response_output_buf, response_output_buf_len, "Error: Invalid argument '%s'", args_str);
        return 1;
    }

    if (toggle_status != 1){
        snprintf(response_output_buf, response_output_buf_len, "Error: Invalid value, Expected Value = 1");
        return 2;
    }


    // Transmit setup commands for the GPS
    // TODO: Verify the set up commands and add the,
    const char *gps_setup_cmds[] = {
        // "unlogall COM1 true\n",
        // "log versionb once\n",
        "log bestxyza ontime 1\n"
    };

    for (size_t i = 0; i < sizeof(gps_setup_cmds) / sizeof(gps_setup_cmds[0]); i++) {
        if (HAL_UART_Transmit(&huart3, (uint8_t *)gps_setup_cmds[i], strlen(gps_setup_cmds[i]), HAL_MAX_DELAY) != HAL_OK) {
            // Transmission error handling
            snprintf(response_output_buf, response_output_buf_len, "Error: GPS UART transmission failed");
            return 3;
        }

        // Adding a delay between each transmission. Need to verify if there is a confirmation for these log commands
        // TODO: Verify if delay is necessary
        HAL_Delay(500);
    }

    // Call GPS Uart Toggle Function
    GPS_set_uart_interrupt_state(toggle_status);

    snprintf(response_output_buf, response_output_buf_len, "GPS interrupt enable and setup completed successfully");

    return 0;
}

/// @brief Telecommand: Transmit a log command to the GPS receiver through UART
/// @param args_str
/// - Arg 0: Log command to be sent to GPS eg "log bestxyza ontime 1" (string)
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, > 0 error
uint8_t TCMDEXEC_gps_send_cmd_receive_response(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    // TODO: Add basic error checks ie: Empty args_str
    if(args_str == NULL || *args_str == "\0"){
        snprintf(response_output_buf, response_output_buf_len,"Error: Empty args_str");
        return 1;
    }
    // TODO: Check if I need to verify this
    // Format of command is: {unlogall/log/unlog/} {log_command} {time_interval_type} {time_interval}

    const size_t gps_cmd_len = strlen(args_str); 

    // Allocate space to receive incoming GPS response.
    const size_t GPS_rx_buffer_max_size = 512;          
    uint16_t GPS_rx_buffer_len = 0;                     
    uint8_t GPS_rx_buffer[GPS_rx_buffer_max_size];      
    memset(GPS_rx_buffer, 0, GPS_rx_buffer_max_size);   // Initialize all elements to 0

    // Send log command to GPS and receive response
    const uint8_t gps_cmd_response = GPS_send_cmd_get_response(args_str, gps_cmd_len, GPS_rx_buffer, 
        GPS_rx_buffer_max_size, GPS_rx_buffer_len);


    // Handle the gps_cmd_response: Perform the error checks
    // TODO: Figure out all the possible responses<OK and <ERROR

    LOG_message(
        LOG_SYSTEM_GPS, 
        LOG_SEVERITY_NORMAL, 
        LOG_SINK_ALL,
        "GPS Response Code: %d",
        gps_cmd_response
    );

    

    return 0;
}