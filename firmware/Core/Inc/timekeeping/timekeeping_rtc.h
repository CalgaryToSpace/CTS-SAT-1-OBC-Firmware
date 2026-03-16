#ifndef INCLUDE_GUARD__TIMEKEEPING_RTC_H
#define INCLUDE_GUARD__TIMEKEEPING_RTC_H

#include "stm32l4xx_hal.h"

#include <time.h>
#include <stdint.h>


void TIME_unix_epoch_time_ms_to_rtc_hal(
    uint64_t unix_ms,
    RTC_TimeTypeDef *time,
    RTC_DateTypeDef *date
);

uint64_t TIME_hal_rtc_to_unix_epoch_time_ms(RTC_DateTypeDef *date, RTC_TimeTypeDef *time);

#endif // INCLUDE_GUARD__TIMEKEEPING_RTC_H
