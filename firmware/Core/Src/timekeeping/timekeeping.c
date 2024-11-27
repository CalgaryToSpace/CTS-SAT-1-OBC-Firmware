#include "timekeeping/timekeeping.h"
#include "debug_tools/debug_uart.h"
#include "transforms/arrays.h"
#include "stm32l4xx_hal.h"
#include "log/log.h"

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

uint32_t TIM_get_current_system_uptime_ms(void)
{
    return HAL_GetTick();
}

/// @brief Use this function in a telecommand, or upon receiving a time update from the GPS.
void TIM_set_current_unix_epoch_time_ms(uint64_t current_unix_epoch_time_ms, TIM_sync_source_t source)
{
    // Determine whether the current sync time is before the last sync time.
    // It would be a warning scenario which makes logs difficult to decipher.
    // Don't log it right away; update the time info, then log after.
    const uint8_t is_this_sync_before_the_last_sync = current_unix_epoch_time_ms < TIM_unix_epoch_time_at_last_time_resync_ms;

    // Update the time.
    TIM_system_uptime_at_last_time_resync_ms = HAL_GetTick();
    TIM_unix_epoch_time_at_last_time_resync_ms = current_unix_epoch_time_ms;
    TIM_last_synchronization_source = source;

    // Convert the time into a string, zero-padded.
    // Format: 13 digits, plus terminator
    GEN_uint64_to_padded_str(
        TIM_unix_epoch_time_at_last_time_resync_ms,
        TIM_EPOCH_DECIMAL_STRING_LEN,
        TIM_unix_epoch_time_at_last_time_resync_ms_str);

    // Log a warning if the current sync time is before the last sync time.
    if (is_this_sync_before_the_last_sync)
    {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "WARNING: setting current time to before the last sync.");
        // TODO: use the logger warning function, and add other data with a format string here.
    }
}

/// @brief Returns the current unix timestamp, in milliseconds
uint64_t TIM_get_current_unix_epoch_time_ms()
{
    return (HAL_GetTick() - TIM_system_uptime_at_last_time_resync_ms) + TIM_unix_epoch_time_at_last_time_resync_ms;
}

/// @brief Returns a computer-friendly timestamp string.
/// @param log_str - Pointer to buffer that stores the log string
/// @param max_len - Maximum length of log_str buffer
/// @detail The string identifies the timestamp of the last time synchronization,
/// the synchronization source (N - none; G - GNSS/GPS; T - telecommand),
/// and the time passed in ms since the last synchronization.
/// Added together, the two numbers represent the current timestamp in ms.
/// Example: "1719169299720+0000042000_N"
void TIM_get_timestamp_string(char *log_str, size_t max_len)
{
    if (max_len < TIM_EPOCH_DECIMAL_STRING_LEN)
    {
        return;
    }
    const char source = TIME_sync_source_enum_to_letter_char(TIM_last_synchronization_source);
    const uint32_t delta_uptime = (TIM_get_current_system_uptime_ms() - TIM_system_uptime_at_last_time_resync_ms);
    snprintf(
        log_str,
        max_len,
        "%s+%010lu_%c",
        TIM_unix_epoch_time_at_last_time_resync_ms_str,
        delta_uptime,
        source);

    return;
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
void TIM_get_timestamp_string_datetime(char *log_str, size_t max_len)
{
    const uint64_t epoch = TIM_get_current_unix_epoch_time_ms();
    const time_t seconds = (time_t)(epoch / 1000U);
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
        delta_uptime);
}

char TIME_sync_source_enum_to_letter_char(TIM_sync_source_t source)
{
    switch (TIM_last_synchronization_source)
    {
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
