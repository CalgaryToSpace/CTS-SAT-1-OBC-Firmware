#include "telecommands/timekeeping_telecommand_defs.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "log/log.h"
#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_commands.h"

#include "timekeeping/timekeeping.h"
#include "eps_drivers/eps_time.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>

uint8_t TCMDEXEC_get_system_time(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    TIM_get_timestamp_string(response_output_buf, response_output_buf_len);
    return 0;
}

/// @brief Set the system time to the provided Unix epoch time in milliseconds
/// @param args_str
/// - Arg 0: Unix epoch time in milliseconds (uint64_t)
/// @return 0 if successful, 1 if error
uint8_t TCMDEXEC_set_system_time(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
  
    uint64_t ms = 0;

    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &ms);
    if (result != 0) {
        return 1;
    }
    TIM_set_current_unix_epoch_time_ms(
        ms, TIM_SOURCE_TELECOMMAND_ABSOLUTE
    );
    snprintf(response_output_buf, response_output_buf_len, "Updated system time");
    return 0;
}

/// @brief Adjust the system time with a signed int
/// @param args_str
/// - Arg 0: Correction time in milliseconds (int64_t). Positive = forward in time, negative = backward in time.
/// @return 0 if successful, 1 if error
uint8_t TCMDEXEC_correct_system_time(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len){

    int64_t correction_time_ms = 0;

    // Convert args_str to signed int and checks if arguement is correct
    if (TCMD_ascii_to_int64(args_str, strlen(args_str), &correction_time_ms) != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Incorrect argument entered. Please enter a signed integer number of milliseconds");
        return 1;
    }

    TIM_set_current_unix_epoch_time_ms(
        TIM_get_current_unix_epoch_time_ms() + correction_time_ms,
        TIM_SOURCE_TELECOMMAND_CORRECTION
    );
    snprintf(response_output_buf, response_output_buf_len, "Updated system time");
    
    if(correction_time_ms>=2000)
    {
        LOG_message(LOG_SYSTEM_ALL, LOG_SEVERITY_WARNING, LOG_SINK_ALL, "Synchronization has changed system time by 2000ms or more. Time deviation was %ld ms.", (uint32_t)correction_time_ms);
    }

    return 0;
}

/// @brief Sync's obc time to eps time (+/- 1 second)
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_set_obc_time_based_on_eps_time(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    EPS_struct_system_status_t status;

    const uint8_t pre_result_status = EPS_CMD_get_system_status(&status);
    if (pre_result_status != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "syncing obc time failed: pre-result status error"
        );
        return 1;
    }

    int64_t obc_time_before_sync_ms = ((int64_t) status.unix_time_sec) * 1000;

    const uint8_t result = EPS_set_obc_time_based_on_eps_time();

    const uint8_t post_result_status = EPS_CMD_get_system_status(&status);
    if (post_result_status != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "syncing obc time failed: post-result status error"
        );
        return 1;
    }

    int64_t obc_time_after_sync_ms = ((int64_t) status.unix_time_sec) * 1000;

    if (result != 0 ) {
        snprintf(response_output_buf, response_output_buf_len,
        "syncing obc time failed");
        return 1;
    }
    snprintf(
        response_output_buf, response_output_buf_len,
        "success syncing obc time"
    );

    LOG_message(LOG_SYSTEM_ALL, LOG_SEVERITY_NORMAL, LOG_SINK_ALL, "CLock synced, OBC time before sync: %lld ms. OBC time after sync: %lld ms. Time difference: %lld", obc_time_before_sync_ms, obc_time_after_sync_ms, obc_time_after_sync_ms-obc_time_before_sync_ms);    

    return 0;
}


/// @brief Sync's eps time to obc time (+/- 1 second)
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_set_eps_time_based_on_obc_time(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    EPS_struct_system_status_t status;

    const uint8_t pre_result_status = EPS_CMD_get_system_status(&status);
    if (pre_result_status != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "syncing eps time failed: pre-result status error"
        );
        return 1;
    }

    int64_t eps_time_before_sync_ms =  ((int64_t) status.unix_time_sec) * 1000;
    
    const uint8_t result = EPS_set_eps_time_based_on_obc_time();
    
    const uint8_t post_result_status = EPS_CMD_get_system_status(&status);
    if (post_result_status != 0) {
        snprintf(        
            response_output_buf, response_output_buf_len,
            "syncing eps time failed: post-result status error"
        );
        return 1;
    }

    int64_t eps_time_after_sync_ms = ((int64_t) status.unix_time_sec) * 1000;
    
    if (result != 0 ) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Syncing eps time failed."
        );
        return 1;
    }
    snprintf(
        response_output_buf, response_output_buf_len,
        "Success syncing eps time."
    );

    LOG_message(LOG_SYSTEM_ALL, LOG_SEVERITY_NORMAL, LOG_SINK_ALL, "CLock synced, EPS time before sync: %lld ms. EPS time after sync: %lld ms. Time difference: %lld", eps_time_before_sync_ms, eps_time_after_sync_ms, eps_time_after_sync_ms-eps_time_before_sync_ms);    

    return 0;
}
