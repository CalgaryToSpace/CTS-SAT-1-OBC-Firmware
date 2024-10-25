#include "eps_drivers/eps_commands.h"
#include "timekeeping/timekeeping.h"
#include "log/log.h"

#include <stdlib.h>


/// @brief Sets the EPS's time, based on the OBC's current time.
/// @return 0 on success, >0 on failure.
uint8_t EPS_set_eps_time_based_on_obc_time() {
    // If the OBC's time is less than 2010-01-01T00:00:00Z, then raise an error.
    // The EPS doesn't like it when you give pre-2000 timestamps.
    if (TIM_get_current_unix_epoch_time_ms() < 1262329200000) {
        return 3;
    }

    EPS_struct_system_status_t status;
    const uint8_t result_status = EPS_CMD_get_system_status(&status);
    if (result_status != 0) {
        return 1;
    }
    
    // +500ms has the same effect as rounding properly in a float division.
    const int32_t delta_seconds = ((TIM_get_current_unix_epoch_time_ms() + 500) / 1000) - ((int64_t) status.unix_time_sec);

    const uint8_t result_sync = EPS_CMD_correct_time(delta_seconds);
    LOG_message(
        LOG_SYSTEM_EPS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "EPS time updated by %ld seconds.",
        delta_seconds
    );

    if (result_sync != 0) {
        return 2;
    }
    return 0;
}

/// @brief Sets the OBC's time, based on the EPS's current time.
/// @return 0 on success, >0 on failure.
uint8_t EPS_set_obc_time_based_on_eps_time() {
    EPS_struct_system_status_t status;
    const uint8_t result_status = EPS_CMD_get_system_status(&status);
    if (result_status != 0) {
        return 1;
    }

    TIM_set_current_unix_epoch_time_ms(
        ((uint64_t) status.unix_time_sec) * 1000,
        TIM_SOURCE_EPS_RTC
    );
    return 0;
}
