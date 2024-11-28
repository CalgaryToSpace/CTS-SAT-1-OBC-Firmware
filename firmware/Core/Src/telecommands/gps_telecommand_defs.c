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