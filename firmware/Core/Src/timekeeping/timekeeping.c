#include "timekeeping/timekeeping.h"
#include "debug_tools/debug_uart.h"
#include "stm32l4xx_hal.h"

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>

uint64_t TIM_unix_epoch_time_at_last_time_resync_ms = 0;
uint32_t TIM_system_uptime_at_last_time_resync_ms = 0;
TIM_sync_source_t TIM_last_synchronization_source = TIM_SOURCE_NONE;

uint32_t TIM_get_current_system_uptime_ms(void) {
    return HAL_GetTick();
}

/// @brief Use this function in a telecommand, or upon receiving a time update from the GPS. 
void TIM_set_current_unix_epoch_time_ms(uint64_t current_unix_epoch_time_ms, TIM_sync_source_t source) {
    // Determine whether the current sync time is before the last sync time.
    // It would be a warning scenario which makes logs difficult to decipher.
    // Don't log it right away; update the time info, then log after.
    const uint8_t is_this_sync_before_the_last_sync = current_unix_epoch_time_ms < TIM_unix_epoch_time_at_last_time_resync_ms;

    // Update the time.
    TIM_unix_epoch_time_at_last_time_resync_ms = current_unix_epoch_time_ms;
    TIM_system_uptime_at_last_time_resync_ms = HAL_GetTick();
    TIM_last_synchronization_source = source;

    // Log a warning if the current sync time is before the last sync time.
    if (is_this_sync_before_the_last_sync) {
        char log_str[TIM_EPOCH_DECIMAL_STRING_LEN];
        TIM_epoch_ms_to_decimal_string(log_str, TIM_EPOCH_DECIMAL_STRING_LEN);
        DEBUG_uart_print_str("WARNING: setting current time to before the last sync.");
        // TODO: use the logger warning function, and add other data with a format string here.
    }
}

/// @brief Returns the current unix timestamp, in milliseconds
uint64_t TIM_get_current_unix_epoch_time_ms() {
    return (HAL_GetTick() - TIM_system_uptime_at_last_time_resync_ms) + TIM_unix_epoch_time_at_last_time_resync_ms;
}

/// @brief Returns a computer-friendly timestamp string. 
/// @param log_str - Pointer to buffer that stores the log string 
/// @param max_len - Maximum length of log_str buffer
/// @detail The string identifies the current Unix time, and the
/// synchronization source (N - none; G - GNSS/GPS; T - telecommand), 
/// and the time passed in ms since the last synchronization.
/// Example: "1719169299720_G_0000042000"
void TIM_get_timestamp_string(char *log_str, size_t max_len) {

    if (max_len < TIM_EPOCH_DECIMAL_STRING_LEN) {
        return;
    }
    char source = TIM_synchronization_source_letter(TIM_last_synchronization_source);
    uint32_t delta_uptime = TIM_get_current_system_uptime_ms() - TIM_system_uptime_at_last_time_resync_ms;
    TIM_epoch_ms_to_decimal_string(log_str, max_len);
    snprintf(log_str + TIM_EPOCH_DECIMAL_STRING_LEN - 1, max_len - TIM_EPOCH_DECIMAL_STRING_LEN, "_%c_%010lu", source, delta_uptime); 

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
void TIM_get_timestamp_string_datetime(char *log_str, size_t max_len) {
    uint64_t epoch = TIM_get_current_unix_epoch_time_ms();
    time_t seconds = (time_t)(epoch/ 1000U);
    uint16_t ms = epoch - 1000U * seconds;
    struct tm *time_info = gmtime(&seconds);

    char source = TIM_synchronization_source_letter(TIM_last_synchronization_source);
    uint32_t delta_uptime = TIM_get_current_system_uptime_ms() - TIM_system_uptime_at_last_time_resync_ms;
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

    return;
}

char TIM_synchronization_source_letter(TIM_sync_source_t source) {

    char source_letter;
    switch (TIM_last_synchronization_source) {
        case TIM_SOURCE_GNSS:
            source_letter = 'G';
            break;
        case TIM_SOURCE_TELECOMMAND:
            source_letter = 'T';
            break;
        case TIM_SOURCE_NONE:
            source_letter = 'N';
            break;
        default:
            source_letter = 'E';
            break;
    }

    return source_letter;
}

/// @brief Efficient conversion of the synchronized epoch in ms to a decimal
/// string
/// @param str - buffer to store result in 
/// @param len - size of result buffer 
/// @detail Strategy: Keep track of the previous large part of ms
/// This will change rarely, and most of the time only the 9 least significant
/// decimal digits need to be updated.
///
/// 100 years from 1 Jan 2024, the value of ms will be 2_019_686_400_000. 
/// For the timestamp string, let's set aside 13 digits of the possible 20 for 
/// an unsigned 64-bit number needed to store ms since the 1970 epoch. 
///
/// We use snprintf to print the least-significant digits of ms on every call.
/// With the "%lu" format string, 32 bits can be printed in one go, i.e. up to
/// the number 4_294_967_295. We will have to restrict printing the 9
/// right-most digits with "%09lu", leaving the remaining digits to be
/// calculated individually.
///
/// The remaining digits to the left will have to be printed only every 
/// 3.4 days.
///
/// Characters of ms_digits[] in order of most significant to least:
///      12 11 10  9        8  7  6  5  4  3  2  1  0 '\0'
/// Array index
///       0  1  2  3        4  5  6  7  8  9 10 11 12 13
/// --  updated at sync  -- |  -- updated every call  -- 
///      or typically 
///     every 3.4 days
void TIM_epoch_ms_to_decimal_string(char *str, size_t len) {

    static uint64_t last_ms_large_part = 0;
    static char ms_digits[TIM_EPOCH_DECIMAL_STRING_LEN] = "0000000000000";

    uint64_t ms = TIM_get_current_unix_epoch_time_ms();
    
    uint32_t ms_small_part = ms % 1000000000;

    // On every call to this function, update the right-most 9 digits
    snprintf(ms_digits + 4, 10, "%09lu", ms_small_part);

    // Return if there is nothing else to do
    uint64_t ms_large_part = ms / 1000000000;
    if (ms_large_part == last_ms_large_part) {
        snprintf(str, len, "%s", ms_digits);
        return;
    }

    // Update the large part, one digit at a time
    uint8_t digit = 0;
    uint64_t working_large_part = ms_large_part;
    for (int8_t d = 3; d >= 0; d--) {
        digit = working_large_part % 10;
        // ASCII '0' is 48 (decimal)
        ms_digits[d] = (char)(48 + digit);
        working_large_part /= 10;
    }
    last_ms_large_part = ms_large_part;
    snprintf(str, len, "%s", ms_digits);
    return;

}

/// @brief  Getter function for the variable 'TIM_system_uptime_at_last_time_resync_ms'
/// @return Returns the variable 'TIM_system_uptime_at_last_time_resync_ms' for other file use.
/// @note This function is mostly intended for "system stats" telecommands and logging.
uint32_t TIM_get_system_uptime_at_last_time_resync_ms() {
    return TIM_system_uptime_at_last_time_resync_ms;
}
