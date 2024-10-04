#include "eps_drivers/eps_commands.h"
#include "timekeeping.h"
uint8_t EPS_TIME_sync_eps_time_to_obc_time() {
    EPS_struct_system_status_t status;
    uint8_t result = EPS_CMD_get_system_status(&status);
    if (result != 0) return result;
    
    int64_t delta_seconds = (int64_t)((float)TIM_get_current_unix_epoch_time_ms() / 1000.0f )
                            -(int64_t)(status.unix_time_sec);
                            
    result = EPS_CMD_correct_time(delta_seconds);
    return result;
}