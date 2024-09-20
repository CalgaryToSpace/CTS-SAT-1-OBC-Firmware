#include "telecommands/telecommand_definitions.h"
#include "uart_handler/uart_handler.h"



/// @brief Telecommand: Enable the interrupt mode for the GPS UART line
/// @param args_str
/// - Arg 0: Toggle status for the GPS_set_uart_interrupt_state function (integer): Should be either 0 or 1
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success
uint8_t TCMDEXEC_gps_set_enabled(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    // Turn on power
    HAL_Delay(1000);

    // Converting the string response to an integer
    char *endptr;
    const uint8_t toggle_status = (uint8_t) (args_str, &endptr, 10);

    // Error checking for the string

    // Transmit setup commands for the GPS

    HAL_Delay(500);
    GPS_set_uart_interrupt_state(toggle_status);

    return 0;
}