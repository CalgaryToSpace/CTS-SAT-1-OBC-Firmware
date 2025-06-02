#include "timekeeping/timekeeping.h"
#include "debug_tools/debug_uart.h"
#include "transforms/arrays.h"
#include "log/log.h"
#include "stm32l4xx_hal.h"

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>

uint64_t TIM_unix_epoch_time_at_last_time_resync_ms = 0;
// 13 digits, plus terminator. Represents TIM_unix_epoch_time_at_last_time_resync_ms. Updated every
// time TIM_unix_epoch_time_at_last_time_resync_ms is updated.
char TIM_unix_epoch_time_at_last_time_resync_ms_str[14] = "0000000000000";
uint32_t TIM_system_uptime_at_last_time_resync_ms = 0;
TIM_sync_source_t TIM_last_synchronization_source = TIM_SOURCE_NONE;

uint32_t TIM_get_current_system_uptime_ms(void) {
    return HAL_GetTick();
}

/// @brief Use this function in a telecommand, or upon receiving a time update from the GNSS. 
void TIM_set_current_unix_epoch_time_ms(uint64_t current_unix_epoch_time_ms, TIM_sync_source_t source) {
    // Determine whether the current sync time is before the last sync time.
    // It would be a warning scenario which makes logs difficult to decipher.
    // Don't log it right away; update the time info, then log after.
    const uint8_t is_this_sync_before_the_last_sync = current_unix_epoch_time_ms < TIM_unix_epoch_time_at_last_time_resync_ms;

    // Compute time before applying sync
    const uint64_t unix_epoch_time_before_sync_ms = TIM_get_current_unix_epoch_time_ms();
    const int64_t correction_time_ms = (int64_t)(current_unix_epoch_time_ms - unix_epoch_time_before_sync_ms);
    
    // Get timestamp string for "before"
    char old_time_str[48];
    TIM_get_timestamp_string_datetime(old_time_str, sizeof(old_time_str));    

    // Update the time.
    TIM_system_uptime_at_last_time_resync_ms = HAL_GetTick();
    TIM_unix_epoch_time_at_last_time_resync_ms = current_unix_epoch_time_ms;
    TIM_last_synchronization_source = source;

    // Convert the time into a string, zero-padded.
    // Format: 13 digits, plus terminator
    GEN_uint64_to_padded_str(
        TIM_unix_epoch_time_at_last_time_resync_ms,
        TIM_EPOCH_DECIMAL_STRING_LEN,
        TIM_unix_epoch_time_at_last_time_resync_ms_str
    );

    // Log a warning if the current sync time is before the last sync time.
    if (is_this_sync_before_the_last_sync) {
        LOG_message(
            LOG_SYSTEM_OBC, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "TIM_set_current_unix_epoch_time_ms: current time is before the last sync (last_resync=%s)",
            TIM_unix_epoch_time_at_last_time_resync_ms_str
        );
    }

    // Log a warning if the time sync changes the time by more than 2 seconds.
    char correction_time_ms_str[21];
    GEN_int64_to_str(correction_time_ms, correction_time_ms_str);
    if(correction_time_ms>=2000 || correction_time_ms<=-2000)
    {
        LOG_message(
            LOG_SYSTEM_OBC, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Synchronization has changed system time by 2000ms or more. Time deviation was %s ms.",
            correction_time_ms_str
        );
    }
    // Get timestamp string for "after"
    char new_time_str[48];
    TIM_get_timestamp_string_datetime(new_time_str, sizeof(new_time_str));    

    // Log both times and the correction
    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Time synchronized. Old time: %s, New time: %s, Clock shift: %s ms.",
        old_time_str, new_time_str, correction_time_ms_str
    );
}

/// @brief Convert the system uptime to a unix epoch time in ms.
/// @param uptime_ms System uptime in ms, as returned by HAL_GetTick().
/// @return The unix epoch time in ms (ms since 1970-01-01).
/// @note This function still works fine even if you store the uptime, resync the system time, and then call this function.
uint64_t TIME_convert_uptime_to_unix_epoch_time_ms(uint32_t uptime_ms) {
    return (uptime_ms - TIM_system_uptime_at_last_time_resync_ms) + TIM_unix_epoch_time_at_last_time_resync_ms;
}

/// @brief Returns the current unix timestamp, in milliseconds
uint64_t TIM_get_current_unix_epoch_time_ms() {
    return TIME_convert_uptime_to_unix_epoch_time_ms(TIM_get_current_system_uptime_ms());
}

/// @brief Returns a computer-friendly timestamp string. 
/// @param dest_str - Pointer to buffer that stores the destination string.
/// @param max_len - Maximum length of log_str buffer.
/// @param uptime_ms - System uptime in ms, as returned by HAL_GetTick().
/// @details The string identifies the timestamp of the last time synchronization,
/// the synchronization source (N - none; G - GNSS/GPS; T - telecommand), 
/// and the time passed in ms since the last synchronization.
/// Added together, the two numbers represent the unix timestamp in ms.
/// Example: "1719169299720+0000042000_N"
void TIME_format_timestamp_str(char dest_str[], size_t max_len, uint32_t uptime_ms) {
    if (max_len < TIM_EPOCH_DECIMAL_STRING_LEN) {
        return;
    }
    const char source = TIME_sync_source_enum_to_letter_char(TIM_last_synchronization_source);
    const uint32_t delta_uptime = (
        uptime_ms - TIM_system_uptime_at_last_time_resync_ms
    );
    snprintf(
        dest_str,
        max_len,
        "%s+%010lu_%c",
        TIM_unix_epoch_time_at_last_time_resync_ms_str,
        delta_uptime,
        source
    );

    return;
}

/// @brief Returns a computer-friendly timestamp string. 
/// @param log_str - Pointer to buffer that stores the log string 
/// @param max_len - Maximum length of log_str buffer
/// @details The string identifies the timestamp of the last time synchronization,
/// the synchronization source (N - none; G - GNSS/GPS; T - telecommand), 
/// and the time passed in ms since the last synchronization.
/// Added together, the two numbers represent the current unix timestamp in ms.
/// Example: "1719169299720+0000042000_N"
void TIM_get_timestamp_string(char *log_str, size_t max_len) {
    TIME_format_timestamp_str(log_str, max_len, TIM_get_current_system_uptime_ms());
}


/// @brief Returns a human-friendly timestamp string 
/// @param log_str - Pointer to buffer that stores the log string 
/// @param max_len - Maximum length of log_str buffer
/// @detail The string identifies the current Unix time, and number of 
/// milliseconds since the time was last synchronized to a reference source.
/// String format:
/// yyyymmddTHHMMSS.sss_X_rrr...
///
/// where
/// 
/// yyyy: year
/// mm: month (01 to 12)
/// dd: day of month (01 to 31)
/// HH: hour (00 to 24)
/// MM: minute (00 to 59)
/// SS: second (00 to 60)
/// sss: millisecond
/// X: last synchronization source, one of N (none), G (GNSS/GPS), T (telecommand)
/// rrr... milliseconds since last time synchronization
/// Example output: 20240623T180132.142_T_420204
void TIM_get_timestamp_string_datetime(char *log_str, size_t max_len) {
    const uint64_t epoch = TIM_get_current_unix_epoch_time_ms();
    const time_t seconds = (time_t)(epoch/ 1000U);
    const uint16_t ms = epoch - 1000U * seconds;
    struct tm *time_info = gmtime(&seconds);

    const char source = TIME_sync_source_enum_to_letter_char(TIM_last_synchronization_source);
    const uint32_t delta_uptime = TIM_get_current_system_uptime_ms() - TIM_system_uptime_at_last_time_resync_ms;
    snprintf(
        log_str, 
        max_len, 
        "%d%02d%02dT%02d%02d%02d.%03u_%c_%lu", 
        time_info->tm_year + 1900, 
        time_info->tm_mon + 1, 
        time_info->tm_mday, 
        time_info->tm_hour, 
        time_info->tm_min, 
        time_info->tm_sec, 
        ms, 
        source,
        delta_uptime
    );
}

char TIME_sync_source_enum_to_letter_char(TIM_sync_source_t source) {
    switch (TIM_last_synchronization_source) {
        case TIM_SOURCE_GNSS:
            return 'G';
        case TIM_SOURCE_TELECOMMAND_ABSOLUTE:
            return 'T';
        case TIM_SOURCE_TELECOMMAND_CORRECTION:
            return 'C';
        case TIM_SOURCE_EPS_RTC:
            return 'E';
        case TIM_SOURCE_NONE:
            return 'N';
    }

    return '?';
}
