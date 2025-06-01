#include "log/log.h"
#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_calculations.h"
#include "timekeeping/timekeeping.h"
#include "stm32/stm32_reboot_reason.h"
#include "eps_drivers/eps_commands.h"
#include "littlefs/littlefs_helper.h"
#include "transforms/arrays.h"
#include "self_checks/complete_self_check.h"
#include "obc_systems/external_led_and_rbf.h"

#include "telecommands/system_telecommand_defs.h"
#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_executor.h"

#include <stdio.h>
#include <string.h>


/// @brief A simple telecommand that responds with "Hello, world!" (log message and TCMD response)
/// @param args_str No arguments expected
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 if successful, >0 if an error occurred (but hello_world can't return an error)
uint8_t TCMDEXEC_hello_world(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Hello, world!"
    );
    snprintf(
        response_output_buf, response_output_buf_len,
        "Hello, world"
    );
    return 0;
}

uint8_t TCMDEXEC_core_system_stats(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    // TODO: Add temperatures (EPS, OBC, antenna, etc.)
    // TODO: Add beacon sent count

    char timestamp_string_ms[20];
    GEN_uint64_to_str(TIM_get_current_unix_epoch_time_ms(), timestamp_string_ms);

    const char* STM32_reset_cause_name = STM32_reset_cause_enum_to_str(STM32_get_reset_cause());

    char time_of_last_tcmd_sent_ms_string[20];
    GEN_uint64_to_str(TCMD_latest_received_tcmd_timestamp_sent, time_of_last_tcmd_sent_ms_string);

    // Get EPS battery percentage as float, or null if error.
    char eps_battery_percent_str[10] = "null";
    EPS_struct_pbu_housekeeping_data_eng_t eps_pbu_data;
    const uint8_t eps_pbu_result = EPS_CMD_get_pbu_housekeeping_data_eng(&eps_pbu_data);
    if (eps_pbu_result == 0) {
        const float battery_percent = EPS_convert_battery_voltage_to_percent(
            eps_pbu_data.battery_pack_info_each_pack[0]
        );
        snprintf(eps_battery_percent_str, sizeof(eps_battery_percent_str), "%0.2f", battery_percent);
    }
    
    snprintf(
        response_output_buf, response_output_buf_len, 
        "{"
        "\"timestamp_ms\":%s,"
        "\"uptime_ms\":%lu,"
        "\"last_resync_ms\":%lu,"
        "\"time_synced_ms_ago\":%lu,"
        "\"time_of_last_tcmd_sent_ms\":%s,"
        "\"total_tcmd_count\":%lu,"
        "\"is_lfs_mounted\":%u,"
        "\"last_time_sync_source\":\"%c\","
        "\"reboot_reason\":\"%s\","
        "\"eps_battery_percent\":%s"
        "}\n",
        timestamp_string_ms, // timestamp_ms
        TIM_get_current_system_uptime_ms(), // uptime_ms
        TIM_system_uptime_at_last_time_resync_ms, // last_resync_ms
        TIM_get_current_system_uptime_ms() - TIM_system_uptime_at_last_time_resync_ms, // time_synced_ms_ago
        time_of_last_tcmd_sent_ms_string, // time_of_last_tcmd_sent_ms
        TCMD_total_tcmd_queued_count, // total_tcmd_count
        LFS_is_lfs_mounted, // is_lfs_mounted
        TIME_sync_source_enum_to_letter_char(TIM_last_synchronization_source), // last_time_sync_source
        STM32_reset_cause_name, // reboot_reason
        eps_battery_percent_str // eps_battery_percent
    ); 

    return 0;
}

uint8_t TCMDEXEC_available_telecommands(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    char *p = response_output_buf;
    uint16_t remaining_space = response_output_buf_len;

    // Start response with header
    snprintf(p, remaining_space, "Available_telecommands\n");
    const uint16_t header_length = strlen(p);
    p += header_length;
    remaining_space -= header_length;

    // Append each telecommand name to the response
    for (uint16_t tcmd_idx = 0; tcmd_idx < TCMD_NUM_TELECOMMANDS; tcmd_idx++) {
        const uint16_t line_length = snprintf(
            p,
            remaining_space,
            "%3u) %s\n",
            tcmd_idx + 1,
            TCMD_telecommand_definitions[tcmd_idx].tcmd_name
        );
        if (line_length >= remaining_space) {
            // Not enough space left to append more telecommands
            break;
        }
        p += line_length;
        remaining_space -= line_length;
    }

    return 0;
}


uint8_t TCMDEXEC_reboot(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    LFS_ensure_unmounted();

    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Rebooting by telecommand request"
    );

    // Delay to flush UART buffer
    HAL_Delay(100);

    NVIC_SystemReset();
    return 0;
}

/// @brief System self-check of all peripherals and systems.
/// @param args_str No arguments expected
/// @param response_output_buf Buffer is filled with a JSON list of strings of the FAILING checks
/// @return 0 regardless; see the response_output_buf for the results of the self-check.
/// @note Output is a JSON list of the failing checks (as strings). Returns 0 regardless.
uint8_t TCMDEXEC_system_self_check_failures_as_json(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    CTS1_system_self_check_result_struct_t self_check_result;
    CTS1_run_system_self_check(&self_check_result);
    CTS1_self_check_struct_TO_json_list(
        self_check_result, response_output_buf, response_output_buf_len,
        0 // Don't show passed checks.
    );
    return 0;
}

/// @brief System self-check of all peripherals and systems.
/// @param args_str No arguments expected
/// @param response_output_buf Buffer is filled with a JSON list of strings of the FAILING checks
/// @return 0 regardless; see the response_output_buf for the results of the self-check.
/// @note Output is a JSON list of the failing checks (as strings). Returns 0 regardless.
uint8_t TCMDEXEC_system_self_check_as_json(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    CTS1_system_self_check_result_struct_t self_check_result;
    CTS1_run_system_self_check(&self_check_result);
    CTS1_self_check_struct_TO_json_list(
        self_check_result, response_output_buf, response_output_buf_len,
        1 // Show passed checks.
    );
    return 0;
}

uint8_t TCMDEXEC_obc_get_rbf_state(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const OBC_rbf_state_enum_t rbf_state = OBC_get_rbf_state();
    const char *rbf_state_str = (rbf_state == OBC_RBF_STATE_FLYING) ? "FLYING" : "BENCH";
    snprintf(
        response_output_buf, response_output_buf_len,
        "{\"rbf_state\":\"%s\"}",
        rbf_state_str
    );
    return 0;
}