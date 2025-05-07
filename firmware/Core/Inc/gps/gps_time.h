#ifndef INCLUDE_GUARD__GPS_TIME_H__
#define INCLUDE_GUARD__GPS_TIME_H__

#include <stdint.h>

/// @brief Synchronizes system time with the GNSS time via PPS
/// @return 0 on success, 1 on failure
uint8_t TIM_handle_PPS_sync(void);

#endif
