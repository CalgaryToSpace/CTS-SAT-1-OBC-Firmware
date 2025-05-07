#include "timekeeping/timekeeping.h"
#include "log/log.h"

#include <stdlib.h>

uint8_t TIM_handle_PPS_sync(void)
{
    uint64_t gps_current_unix_time_ms;

    if (GNSS_get_unix_time_ms(&gps_current_unix_time_ms))
    {
        TIM_set_current_unix_epoch_time_ms(gps_current_unix_time_ms, TIM_SOURCE_GNSS);
        return 0; // success
    }
    return 1; // failure to get GNSS time
}
