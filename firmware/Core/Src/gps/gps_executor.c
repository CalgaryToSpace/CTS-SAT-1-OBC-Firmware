#include "gps/gps_executor.h"
#include "gps/gps_args_helpers.h"
#include "uart_handler/uart_handler.h"
#include "main.h"

#include <stdio.h>
#include <stdint.h>

/// @brief Transmit a log command to the GPS receiver through the GPS UART line
/// @param gps_log_command Log Command to be transmitted to the GPS receiver (string)
/// @return 0 on success, > 0 error
uint8_t GPS_TCMD_transmit_log_command(const char *gps_log_command) {

    // Check the gps log starts with the appropriate log prefix
    const uint8_t gps_prefix_check_result = GPS_check_starts_with_log_prefix(gps_log_command);

    if(gps_prefix_check_result == 1){
        // Error: Invalid gps log prefix
        return 1;
    }

    // Transmit gps log command to the gps receiver
    // TODO: Check if there are potentially other checks to add
    HAL_StatusTypeDef transmit_status = HAL_UART_Transmit(UART_gps_port_handle, (uint8_t*)gps_log_command, sizeof(gps_log_command),HAL_MAX_DELAY);

    // Check HAL transmit status
    if(transmit_status != HAL_OK){
        return 1;
    }

    // TODO: Add check if the gps receiver responded with '<OK\n\r[COM1]'
    // This does not happen instantenously. Need to figure out where it will go.


    return 0; 
}
