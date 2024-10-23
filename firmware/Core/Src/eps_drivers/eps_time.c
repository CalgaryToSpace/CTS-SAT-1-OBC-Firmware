#include "eps_drivers/eps_commands.h"
#include "timekeeping/timekeeping.h"
#include "log/log.h"

#include <stdlib.h>

/// @brief If the absolute EPS time delta between the OBC and the EPS is greater than this value,
///        the EPS will be updated to match the OBC's time.
uint32_t EPS_min_time_delta_to_update_eps_time_sec = 5;


/// @brief Sets the EPS's time, based on the OBC's current time.
/// @return 0 on success, >0 on failure.
uint8_t EPS_set_eps_time_to_obc_time() {
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
    
    const int32_t delta_seconds = (TIM_get_current_unix_epoch_time_ms() / 1000) - ((int64_t) status.unix_time_sec);

    // No need to update the time if the delta is less than the minimum value.
    // This prevents unnecessary time updates and jitter.
    if (abs(delta_seconds) < EPS_min_time_delta_to_update_eps_time_sec) {
        return 0;
    }

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
