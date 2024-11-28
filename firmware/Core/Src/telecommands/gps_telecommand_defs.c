#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"
#include "telecommands/telecommand_executor.h"
#include "debug_tools/debug_uart.h"
#include "telecommands/gps_telecommand_defs.h"
#include "gps/gps_pps_interrupt.h" 

#include <string.h>
#include <stdio.h>

#include "stm32l4xx_hal.h"

/// @brief Telecommand: Delete all agendas
/// @param args_str No arguments needed
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, 1 gps is off.
uint8_t TCMDEXEC_gps_set_enabled_pps_tracking(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
        // Check if EXTI is active
        if (get_gps_pps_enabled()) {
            // Disable EXTI tracking
            Disable_GPIO_EXTI();
            set_gps_pps_disabled();
            snprintf(
                response_output_buf, response_output_buf_len,
                "PPS tracking disabled");
            return 0;
        } 
        // Check if EXTI is inactive
        else if (!get_gps_pps_enabled()) {
            // Enable EXTI tracking
            GPIO_EXTI_Init();
            set_gps_pps_enabled();
            snprintf(
                response_output_buf, response_output_buf_len,
                "PPS tracking enabled");
            return 0;
        }
    return 1;
}