#include "telecommands/timekeeping_telecommand_defs.h"
#include "telecommand_exec/telecommand_args_helpers.h"

#include "timekeeping/timekeeping.h"
#include "eps_drivers/eps_time.h"
#include "gnss_receiver/gnss_time.h"
#include "uart_handler/uart_handler.h"

#include <stdio.h>
#include <string.h>

uint8_t TCMDEXEC_get_system_time(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    TIME_get_current_timestamp_str(response_output_buf, response_output_buf_len);
    return 0;
}


/// @brief Set the system time to the provided Unix epoch time in milliseconds
/// @param args_str
/// - Arg 0: Unix epoch time in milliseconds (uint64_t)
/// @return 0 if successful, 1 if error
/// @note This function accounts for the unpredictable delay between telecommand reception and execution.
uint8_t TCMDEXEC_set_system_time(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    uint64_t ms = 0;

    const uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &ms);
    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Invalid system time argument");
        return 1;
    }

    // Apply correction between telecommand reception and execution.
    // You could consider this a "hack" because of the random/unpredictable 0-250ms delay
    // in executing queued telecommands.

    // First, find whether it came from AX100 or Umbilical (based on last write/rx time),
    // and pick that last time. That is, select the max of these two values.
    const uint32_t tcmd_received_at_uptime_ms = (
        UART_telecommand_last_write_time_ms > UART_ax100_last_write_time_ms ?
        UART_telecommand_last_write_time_ms : UART_ax100_last_write_time_ms
    );

    // Move the reported "current time" forward by the delay between telecommand reception and execution.
    ms += HAL_GetTick() - tcmd_received_at_uptime_ms;

    TIME_set_current_unix_epoch_time_ms(
        ms, TIME_SYNC_SOURCE_TELECOMMAND_ABSOLUTE
    );
    snprintf(response_output_buf, response_output_buf_len, "Updated system time");

    return 0;
}


/// @brief Set the system time to the provided Unix epoch time in milliseconds.
/// @param args_str
/// - Arg 0: Unix epoch time in milliseconds (uint64_t)
/// @return 0 if successful, 1 if error
/// @note This function is the naive/basic implementation that doesn't account for a delay
///       between telecommand reception and execution.
uint8_t TCMDEXEC_set_system_time_approx(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    uint64_t ms = 0;

    const uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &ms);
    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Invalid system time argument");
        return 1;
    }

    TIME_set_current_unix_epoch_time_ms(
        ms, TIME_SYNC_SOURCE_TELECOMMAND_ABSOLUTE
    );
    snprintf(response_output_buf, response_output_buf_len, "Updated system time");

    return 0;
}



/// @brief Adjust the system time by a correction offset in ms.
/// @param args_str
/// - Arg 0: Correction time in milliseconds (int64_t). Positive = forward in time, negative = backward in time.
/// @return 0 if successful, 1 if error
/// @note Math: new_system_time_ms = current_system_time_ms + correction_time_ms
uint8_t TCMDEXEC_correct_system_time(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    int64_t correction_time_ms = 0;

    // Convert args_str to signed int and checks if arguement is correct
    if (TCMD_ascii_to_int64(args_str, strlen(args_str), &correction_time_ms) != 0) {
        snprintf(response_output_buf, response_output_buf_len, 
            "Incorrect argument entered. Please enter a signed integer number of milliseconds"
        );
        return 1;
    }

    TIME_set_current_unix_epoch_time_ms(
        TIME_get_current_unix_epoch_time_ms() + correction_time_ms,
        TIME_SYNC_SOURCE_TELECOMMAND_CORRECTION
    );
    snprintf(response_output_buf, response_output_buf_len, "Updated system time");
    
    return 0;
}

/// @brief Sync's obc time to eps time (+/- 1 second)
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_set_obc_time_based_on_eps_time(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    const uint8_t result = EPS_set_obc_time_based_on_eps_time();
    if (result != 0 ) {
        snprintf(response_output_buf, response_output_buf_len,
            "Syncing obc time failed"
        );
        return 1;
    }
    snprintf(response_output_buf, response_output_buf_len,
        "Success syncing obc time"
    );
    return 0;
}

/// @brief Sync OBC time based on the GNSS time report (+/- ~100ms, based on system latency).
/// @param args_str No arguments.
/// @return 0 on success, >0 on failure.
/// @note Requires that the GNSS receiver's EPS channel is already powered on, and that a time fix is ready.
uint8_t TCMDEXEC_set_obc_time_based_on_gnss_time(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint8_t result = GNSS_set_obc_time_based_on_gnss_time_uart();
    if (result != 0 ) {
        snprintf(response_output_buf, response_output_buf_len,
            "Syncing obc time based on gnss time failed"
        );
        return result;
    }
    snprintf(response_output_buf, response_output_buf_len,
        "Success syncing obc time based on gnss time"
    );
    return 0;
}

/// @brief Corrects the OBC time based on the GNSS PPS signal. Very precise.
/// @param args_str No arguments.
/// @return 0 on success, >0 on failure.
/// @note Requires that the GNSS receiver's EPS channel is already powered on, and that a time fix is ready.
/// @note Requires an accurate GNSS time fix, and requires that `set_obc_time_based_on_gnss_time` has been
///     run recently (or that the OBC time is correct within 500ms based on another time source).
uint8_t TCMDEXEC_set_obc_time_based_on_gnss_pps(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    return GNSS_set_obc_time_based_on_gnss_pps();
}


/// @brief Sync's eps time to obc time (+/- 1 second)
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_set_eps_time_based_on_obc_time(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    const uint8_t result = EPS_set_eps_time_based_on_obc_time();
    if (result != 0 ) {
        snprintf(response_output_buf, response_output_buf_len,
            "Syncing eps time failed."
        );
        return 1;
    }
    snprintf(response_output_buf, response_output_buf_len,
        "Success syncing eps time."
    );
    return 0;
}
