#include "telecommands/timekeeping_telecommand_defs.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "log/log.h"
#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_commands.h"
#include "transforms/arrays.h"
#include "timekeeping/timekeeping.h"
#include "eps_drivers/eps_time.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

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
        snprintf(
            response_output_buf, response_output_buf_len,
            "Incorrect argument entered. Please enter a signed integer number of milliseconds"
        );
        return 1;
    }

    // Convert limits to string for printing
    char MIN_BUFFER[21];
    char MAX_BUFFER[21];
    GEN_int64_to_str(INT64_MIN, MIN_BUFFER);
    GEN_int64_to_str(INT64_MIN, MAX_BUFFER);

    if(correction_time_ms>INT32_MAX || correction_time_ms<INT32_MIN)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Correction time argument out of bounds: Enter correction time larger than %s and smaller than %s",
            MIN_BUFFER, MAX_BUFFER
        );
        return 1;
    }
    TIM_set_current_unix_epoch_time_ms(
        TIM_get_current_unix_epoch_time_ms() + correction_time_ms,
        TIM_SOURCE_TELECOMMAND_CORRECTION
    );
    snprintf(response_output_buf, response_output_buf_len, "Updated system time");
    
    // Convert correction time in ms to string for printing
    char buffer[21];
    GEN_int64_to_str(correction_time_ms, buffer);
    if(correction_time_ms>=2000 || correction_time_ms<=-2000)
    {
        LOG_message(
            LOG_SYSTEM_ALL, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Synchronization has changed system time by 2000ms or more. Time deviation was %s ms.",
            buffer
        );
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

    int32_t obc_time_before_sync_sec= status.unix_time_sec;

    const uint8_t result = EPS_set_obc_time_based_on_eps_time();

    const uint8_t post_result_status = EPS_CMD_get_system_status(&status);
    if (post_result_status != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "syncing obc time failed: post-result status error"
        );
        return 1;
    }

    int32_t obc_time_after_sync_sec= status.unix_time_sec * 1000;

    if (result != 0 ) {
        snprintf(response_output_buf, response_output_buf_len,
        "syncing obc time failed");
        return 1;
    }
    snprintf(
        response_output_buf, response_output_buf_len,
        "success syncing obc time"
    );

    LOG_message(
        LOG_SYSTEM_ALL, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "CLock synced, OBC time before sync: %ld s. OBC time after sync: %ld s. Time difference: %ld s",
        obc_time_before_sync_sec, obc_time_after_sync_sec, obc_time_after_sync_sec-obc_time_before_sync_sec
    );    

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

    int32_t eps_time_before_sync_sec=  status.unix_time_sec;
    
    const uint8_t result = EPS_set_eps_time_based_on_obc_time();
    
    const uint8_t post_result_status = EPS_CMD_get_system_status(&status);
    if (post_result_status != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "syncing eps time failed: post-result status error"
        );
        return 1;
    }

    int32_t eps_time_after_sync_sec= status.unix_time_sec;
    
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

    LOG_message(
        LOG_SYSTEM_ALL, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "CLock synced, EPS time before sync: %ld s. EPS time after sync: %ld s. Time difference: %ld s",
        eps_time_before_sync_sec, eps_time_after_sync_sec, eps_time_after_sync_sec-eps_time_before_sync_sec
    );    

    return 0;
}
