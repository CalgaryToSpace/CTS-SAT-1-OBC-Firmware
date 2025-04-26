#include "boom_deploy_drivers/boom_deploy_drivers.h"

#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_args_helpers.h"

#include "eps_drivers/eps_commands.h"
#include "eps_drivers/eps_types_to_json.h"
#include "eps_drivers/eps_types.h"

#include "telecommands/boom_deploy_telecommand_defs.h"

#include "log/log.h"

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// @brief Activate the burn wire for the specified duration.
/// @param args_str 
/// - Arg 0: Which boom deployment channel (1=CTRL_1 or 2=CTRL_2), or 0 for both.
/// - Arg 1: Duration (milliseconds), less than 14.1 seconds (<=14100 ms)
/// @return 0 on success, 1-2 on parsing error, 20 on EPS failure
/// @note Does not enable the EPS channel. You must manually enable the EPS channel before
///     calling this function.
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

    uint64_t duration_ms_u64;
    const uint8_t arg1_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &duration_ms_u64);

    if (arg1_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing arguments. Return codes: arg0=%d",
            arg1_result);
        return 3;
    }

    if (duration_ms_u64 > 14100) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Duration must be <= 14.1 seconds.");
        return 4;
    }

    const uint32_t duration_ms = (uint32_t)duration_ms_u64;
    const uint32_t start_time_ms = HAL_GetTick();
    

    if (channel_u64 == 0) { // 0 = Both channels
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_NORMAL,
            LOG_SINK_ALL,
            "Enabling boom deploy ctrl on both channels for %lu ms.",
            duration_ms
        );

        BOOM_set_burn_enabled(1, 1);
        BOOM_set_burn_enabled(2, 1);
    }
    else if ((channel_u64 == 1) || (channel_u64 == 2)) {
        BOOM_set_burn_enabled((uint8_t)channel_u64, 1);

        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_NORMAL,
            LOG_SINK_ALL,
            "Enabling boom deploy ctrl on BOOM_CTRL_%d for %lu ms.",
            (uint8_t)channel_u64,
            duration_ms
        );
    }
    else {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Fail: Channel must be 1 or 2."
        );
        return 2;
    }

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

        // Log EPS power usage.
        char eps_power_usage_json[200];
        EPS_struct_pdu_housekeeping_data_eng_t eps_data;
        const uint8_t eps_pdu_cmd_result = EPS_CMD_get_pdu_housekeeping_data_eng(&eps_data);

        if (eps_pdu_cmd_result != 0) {
            LOG_message(
                LOG_SYSTEM_BOOM, LOG_SEVERITY_WARNING,
                LOG_SINK_ALL,
                "Error getting EPS power usage. Error code: %d",
                eps_pdu_cmd_result
            );
            continue;
        }

        const uint8_t to_json_result = EPS_vpid_eng_TO_json(
            &eps_data.vip_each_channel[EPS_CHANNEL_12V_BOOM],
            eps_power_usage_json, sizeof(eps_power_usage_json)
        );
        if (to_json_result != 0) {
            LOG_message(
                LOG_SYSTEM_BOOM, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                "Error converting EPS data to JSON. Error code: %d",
                to_json_result
            );
            continue;
        }

        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "EPS power usage: %s",
            eps_power_usage_json
        );
    }

    BOOM_disable_all_burns();

    LOG_message(
        LOG_SYSTEM_BOOM, LOG_SEVERITY_NORMAL,
        LOG_SINK_ALL,
        "Boom deploy ctrl disabled after %lu ms.",
        duration_ms
    );

    return 0;
}
