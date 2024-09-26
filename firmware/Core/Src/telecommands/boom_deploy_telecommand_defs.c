#include "boom_deploy_drivers/boom_deploy_drivers.h"

#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_args_helpers.h"

#include "telecommands/boom_deploy_telecommand_defs.h"

#include "log/log.h"

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// @brief Activate the burn wire for the specified duration.
/// @param args_str 
/// - Arg 0: Duration (milliseconds), less than 10 seconds
/// @return 0 on success, 1-2 on parsing error, 10 on PGOOD fail, 20 on EPS failure
uint8_t TCMDEXEC_boom_deploy_timed(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    uint64_t duration_ms_u64;
    const uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &duration_ms_u64);

    if (arg0_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing arguments. Return codes: arg0=%d",
            arg0_result);
        return 1;
    }

    if (duration_ms_u64 > 10000) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Duration must be less than 10 seconds.");
        return 2;
    }

    const uint32_t duration_ms = (uint32_t)duration_ms_u64;
    const uint32_t start_time_ms = HAL_GetTick();
    
    BOOM_set_burn_enabled(1);

    while (1) {
        const uint32_t current_running_time_ms = HAL_GetTick() - start_time_ms;
        if (current_running_time_ms >= duration_ms) {
            break;
        }

        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Boom Burning. PGOOD=%s",
            BOOM_get_pgood_status() ? "good" : "bad"
        );

        // if (BOOM_get_pgood_status() == 0) {
        //     BOOM_set_burn_enabled(0);
        //     return 10;
        // }

        // Delay for a short duration at the start, then longer after.
        if (current_running_time_ms < 250) {
            HAL_Delay(25);
        }
        else {
            HAL_Delay(duration_ms / 10);
        }
        // TODO: implement the EPS monitoring logic.
    }

    BOOM_set_burn_enabled(0);
    return 0;
}
