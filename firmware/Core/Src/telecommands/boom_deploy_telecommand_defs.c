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
/// - Arg 0: Which boom deployment channel (0 or 1)
/// - Arg 1: Duration (milliseconds), less than 12.5 seconds (<12500 ms)
/// @return 0 on success, 1-2 on parsing error, 20 on EPS failure
uint8_t TCMDEXEC_boom_deploy_timed(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    uint64_t channel_u64;
    const uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &channel_u64);
    if (arg0_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Fail: Error parsing arg 0 (channel). Err=%d",
            arg0_result
        );
        return 1;
    }
    if (channel_u64 > 1) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Fail: Channel must be 0 or 1."
        );
        return 2;
    }

    uint64_t duration_ms_u64;
    const uint8_t arg1_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &duration_ms_u64);

    if (arg1_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing arguments. Return codes: arg0=%d",
            arg1_result);
        return 3;
    }

    if (duration_ms_u64 > 12500) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Duration must be less than 12.5 seconds.");
        return 4;
    }

    const uint32_t duration_ms = (uint32_t)duration_ms_u64;
    const uint8_t channel = (uint8_t)channel_u64;
    const uint32_t start_time_ms = HAL_GetTick();
    
    BOOM_set_burn_enabled(channel, 1);

    while (1) {
        const uint32_t current_running_time_ms = HAL_GetTick() - start_time_ms;
        if (current_running_time_ms >= duration_ms) {
            break;
        }

        // Delay for a short duration at the start, then longer after.
        if (current_running_time_ms < 250) {
            HAL_Delay(25);
        }
        else {
            HAL_Delay(duration_ms / 10);
        }

        // TODO: implement the EPS monitoring/logging.
        // LOG_message(
        //     LOG_SYSTEM_BOOM, LOG_SEVERITY_WARNING, LOG_SINK_ALL
        // );
    }

    BOOM_disable_all_burns();

    return 0;
}
