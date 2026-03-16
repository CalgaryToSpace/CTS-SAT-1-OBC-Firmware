#include "timekeeping/timekeeping.h"

#include "timekeeping/timekeeping_rtc.h"
#include "debug_tools/debug_uart.h"
#include "transforms/arrays.h"
#include "log/log.h"
#include "stm32l4xx_hal.h"

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

extern RTC_HandleTypeDef hrtc;

uint64_t TIME_unix_epoch_time_at_last_time_resync_ms = 0;
// 13 digits, plus terminator. Represents TIME_unix_epoch_time_at_last_time_resync_ms. Updated every
// time TIME_unix_epoch_time_at_last_time_resync_ms is updated.
static char TIME_unix_epoch_time_at_last_time_resync_ms_str[14] = "0000000000000";
uint32_t TIME_system_uptime_at_last_time_resync_ms = 0;
TIME_sync_source_enum_t TIME_last_synchronization_source = TIME_SYNC_SOURCE_NONE;

uint32_t TIME_get_current_system_uptime_ms(void) {
    return HAL_GetTick();
}


void TIME_set_current_unix_epoch_time_ms(
    uint64_t current_unix_epoch_time_ms,
    TIME_sync_source_enum_t source
) {
    const uint64_t unix_epoch_time_before_sync_ms =
        TIME_get_current_unix_epoch_time_ms();

    const uint8_t is_this_sync_before_the_last_sync =
        current_unix_epoch_time_ms < TIME_unix_epoch_time_at_last_time_resync_ms;

    const int64_t correction_time_ms =
        (int64_t)(current_unix_epoch_time_ms - unix_epoch_time_before_sync_ms);

    char old_time_str[48];
    TIME_get_current_utc_datetime_str(old_time_str, sizeof(old_time_str));

    // RTC always gets set to the top of a second, so we must delay to the top of a second here.
    const uint16_t ms = current_unix_epoch_time_ms % 1000;
    const uint16_t delay_to_next_second = (ms == 0) ? 0 : (1000 - ms);
    if (delay_to_next_second > 0) {
        HAL_Delay(delay_to_next_second);
    }

    // Convert unix ms to RTC date/time.
    RTC_TimeTypeDef rtc_time = {0};
    RTC_DateTypeDef rtc_date = {0};
    TIME_unix_epoch_time_sec_to_rtc_hal(
        (current_unix_epoch_time_ms + delay_to_next_second) / 1000,
        &rtc_time, &rtc_date
    );

    // Order here matters! Must do Time then Date.
    HAL_RTC_SetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);

    // Update sync tracking (for logs).
    TIME_system_uptime_at_last_time_resync_ms = HAL_GetTick();
    TIME_unix_epoch_time_at_last_time_resync_ms = current_unix_epoch_time_ms;
    TIME_last_synchronization_source = source;

    GEN_uint64_to_padded_str(
        TIME_unix_epoch_time_at_last_time_resync_ms,
        TIME_EPOCH_DECIMAL_STRING_LEN,
        TIME_unix_epoch_time_at_last_time_resync_ms_str
    );

    if (is_this_sync_before_the_last_sync) {
        LOG_message(
            LOG_SYSTEM_OBC, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "TIME_set_current_unix_epoch_time_ms: current time is before the last sync (last_resync=%s)",
            TIME_unix_epoch_time_at_last_time_resync_ms_str
        );
    }

    char correction_time_ms_str[21];
    GEN_int64_to_str(correction_time_ms, correction_time_ms_str);

    if (correction_time_ms >= 2000 || correction_time_ms <= -2000) {
        LOG_message(
            LOG_SYSTEM_OBC, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Synchronization has changed system time by 2000ms or more. Time deviation was %s ms.",
            correction_time_ms_str
        );
    }

    char new_time_str[48];
    TIME_get_current_utc_datetime_str(new_time_str, sizeof(new_time_str));

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
    return (uptime_ms - TIME_system_uptime_at_last_time_resync_ms) + TIME_unix_epoch_time_at_last_time_resync_ms;
}

/// @brief Returns the current unix timestamp, in milliseconds.
uint64_t TIME_get_current_unix_epoch_time_ms() {
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    // Note: Must fetch time BEFORE date!
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

    return TIME_hal_rtc_to_unix_epoch_time_ms(&date, &time);
}

/// @brief Returns a computer-friendly timestamp string, ideal for chronological ordering. 
/// @param dest_str - Pointer to buffer that stores the destination string.
/// @param dest_str_size - Maximum length of dest_str buffer.
/// @param uptime_ms - System uptime in ms, as returned by HAL_GetTick().
/// @details The string identifies the timestamp of the last time synchronization,
/// the synchronization source (N - none; G - GNSS/GPS; T - telecommand), 
/// and the time passed in ms since the last synchronization.
/// Added together, the two numbers represent the unix timestamp in ms.
/// @example Example: "1719169299720+0000042000_N"
/// @note See the `docs/` folder for more information on this specific format.
void TIME_format_timestamp_str(
    char dest_str[], size_t dest_str_size,
    uint32_t uptime_ms, TIME_sync_source_enum_t sync_source
) {
    if (dest_str_size < TIME_EPOCH_DECIMAL_STRING_LEN) {
        return;
    }
    const char source = TIME_sync_source_enum_to_letter_char(sync_source);
    const uint32_t delta_uptime = (
        uptime_ms - TIME_system_uptime_at_last_time_resync_ms
    );
    snprintf(
        dest_str,
        dest_str_size,
        "%s+%010lu_%c",
        TIME_unix_epoch_time_at_last_time_resync_ms_str,
        delta_uptime,
        source
    );

    return;
}

/// @brief Returns a computer-friendly timestamp string. 
/// @param dest_str - Pointer to buffer that stores the log string 
/// @param dest_str_size - Maximum length of dest_str buffer
/// @details The string identifies the timestamp of the last time synchronization,
/// the synchronization source (N - none; G - GNSS/GPS; T - telecommand), 
/// and the time passed in ms since the last synchronization.
/// Added together, the two numbers represent the current unix timestamp in ms.
/// Example: "1719169299720+0000042000_N"
void TIME_get_current_timestamp_str(char *dest_str, size_t dest_str_size) {
    TIME_format_timestamp_str(
        dest_str, dest_str_size,
        TIME_get_current_system_uptime_ms(),
        TIME_last_synchronization_source
    );
}


/// @brief Returns a human-friendly UTC datetime string in roughly ISO 8601 format.
/// @param dest_str - Pointer to buffer that stores the log string 
/// @param dest_str_size - Maximum length of dest_str buffer
/// @details The string identifies the current Unix time, and number of 
/// milliseconds since the time was last synchronized to a reference source.
/// String format:
/// yyyy-mm-ddTHHMMSS.sss_X
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
/// @example Example output: 2024-06-23T180132.142_T
void TIME_format_utc_datetime_str(
    char *dest_str, size_t dest_str_size,
    uint64_t timestamp_ms, TIME_sync_source_enum_t sync_source
) {
    const time_t seconds = (time_t)(timestamp_ms / 1000U);
    const uint16_t ms = timestamp_ms % 1000U;
    struct tm *time_info = gmtime(&seconds);

    const char sync_source_char = TIME_sync_source_enum_to_letter_char(sync_source);
    snprintf(
        dest_str, 
        dest_str_size, 
        "%d-%02d-%02dT%02d%02d%02d.%03uZ_%c",
        time_info->tm_year + 1900, 
        time_info->tm_mon + 1, 
        time_info->tm_mday, 
        time_info->tm_hour, 
        time_info->tm_min, 
        time_info->tm_sec, 
        ms, 
        sync_source_char
    );
}


/// @brief Returns a human-friendly UTC datetime string in roughly ISO 8601 format, without milliseconds nor sync source.
/// @param dest_str - Pointer to buffer that stores the log string 
/// @param dest_str_size - Maximum length of dest_str buffer
/// @details The string identifies the current Unix time, and number of 
/// milliseconds since the time was last synchronized to a reference source.
/// String format:
/// yyyy-mm-ddTHHMMSSZ
/// @example Example output: 2024-06-23T180132Z
/// @note This function is very useful for log file names.
void TIME_format_utc_datetime_str_no_ms(
    char *dest_str, size_t dest_str_size,
    uint64_t timestamp_ms
) {
    const time_t seconds = (time_t)(timestamp_ms / 1000U);
    struct tm *time_info = gmtime(&seconds);

    snprintf(
        dest_str, 
        dest_str_size, 
        "%d-%02d-%02dT%02d%02d%02dZ",
        time_info->tm_year + 1900, 
        time_info->tm_mon + 1, 
        time_info->tm_mday, 
        time_info->tm_hour, 
        time_info->tm_min, 
        time_info->tm_sec
    );
}


/// @brief Returns a human-friendly UTC datetime string in roughly ISO 8601 format.
/// @param dest_str - Pointer to buffer that stores the log string 
/// @param dest_str_size - Maximum length of dest_str buffer
/// @details The string identifies the current Unix time, and number of 
/// milliseconds since the time was last synchronized to a reference source.
/// String format:
/// yyyy-mm-ddTHHMMSS.sss_X
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
/// @example Example output: 2024-06-23T180132.142_T
void TIME_get_current_utc_datetime_str(char *dest_str, size_t dest_str_size) {
    TIME_format_utc_datetime_str(
        dest_str, dest_str_size,
        TIME_get_current_unix_epoch_time_ms(),
        TIME_last_synchronization_source
    );
}


/// @brief Returns a human-friendly UTC datetime string in roughly ISO 8601 format, without milliseconds nor sync source.
/// @param dest_str - Pointer to buffer that stores the log string 
/// @param dest_str_size - Maximum length of dest_str buffer
/// @details The string identifies the current Unix time, and number of 
/// milliseconds since the time was last synchronized to a reference source.
/// String format:
/// yyyy-mm-ddTHHMMSSZ
/// @example Example output: 2024-06-23T180132Z
/// @note This function is very useful for log file names.
void TIME_get_current_utc_datetime_str_no_ms(char *dest_str, size_t dest_str_size) {
    TIME_format_utc_datetime_str_no_ms(
        dest_str, dest_str_size,
        TIME_get_current_unix_epoch_time_ms()
    );
}


char TIME_sync_source_enum_to_letter_char(TIME_sync_source_enum_t source) {
    switch (source) {
        case TIME_SYNC_SOURCE_GNSS_UART:
            return 'G';
        case TIME_SYNC_SOURCE_GNSS_PPS:
            return 'P';
        case TIME_SYNC_SOURCE_TELECOMMAND_ABSOLUTE:
            return 'T';
        case TIME_SYNC_SOURCE_TELECOMMAND_CORRECTION:
            return 'C';
        case TIME_SYNC_SOURCE_EPS_RTC:
            return 'E';
        case TIME_SYNC_SOURCE_NONE:
            return 'N';
    }

    return '?';
}
