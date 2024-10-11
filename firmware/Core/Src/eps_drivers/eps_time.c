#include "eps_drivers/eps_commands.h"
#include "timekeeping/timekeeping.h"

/**
 * @brief Sync's eps time to obc time (+/- 1 second)
 * @return 0 on success, > 0 on failure.
 */
uint8_t EPS_TIME_sync_eps_time_to_obc_time() {
    EPS_struct_system_status_t status;
    uint8_t result = EPS_CMD_get_system_status(&status);
    if (result != 0) {
        return 1;
    }
    
    int64_t delta_ms = TIM_get_current_unix_epoch_time_ms() - ((uint64_t)status.unix_time_sec * 1000);
    int32_t delta_seconds = delta_ms / 1000;
                            
    result = EPS_CMD_correct_time(delta_seconds);
    if (result != 0) {
        return 2;
    }
    return 0;
}