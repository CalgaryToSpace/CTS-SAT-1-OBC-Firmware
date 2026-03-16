#include "timekeeping/timekeeping_rtc.h"

#include "debug_tools/debug_uart.h"
#include "transforms/arrays.h"
#include "log/log.h"
#include "stm32l4xx_hal.h"

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

extern RTC_HandleTypeDef hrtc;

#define SECONDS_PER_DAY 86400

static const uint16_t days_before_month[12] =
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};


static uint8_t is_leap_year(uint16_t year) {
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}


static int64_t utc_tm_to_unix_seconds(const struct tm *tm) {
    int year = tm->tm_year + 1900;
    int month = tm->tm_mon;
    int day = tm->tm_mday - 1;

    int64_t days = 0;

    for (int y = 1970; y < year; y++) {
        days += is_leap_year(y) ? 366 : 365;
    }

    days += days_before_month[month];

    if (month >= 2 && is_leap_year(year)) {
        days += 1;
    }

    days += day;

    return (((days * 24 + tm->tm_hour) * 60 + tm->tm_min) * 60 + tm->tm_sec);
}

static inline uint32_t rtc_subseconds_to_ms(const RTC_TimeTypeDef *time) {
    const uint32_t ticks = time->SecondFraction - time->SubSeconds;
    const uint32_t denom = time->SecondFraction + 1;

    return (ticks * 1000ULL + denom / 2) / denom;  // rounded
}


static inline uint32_t ms_to_rtc_subseconds(uint32_t ms, uint32_t second_fraction) {
    const uint32_t denom = second_fraction + 1;

    uint32_t ticks = (ms * denom + 500) / 1000;  // rounded

    if (ticks > second_fraction) {
        ticks = second_fraction;
    }

    return second_fraction - ticks;
}


uint64_t TIME_hal_rtc_to_unix_epoch_time_ms(RTC_DateTypeDef *date, RTC_TimeTypeDef *time) {
    struct tm t = {0};
    t.tm_year = date->Year + 100;
    t.tm_mon  = date->Month - 1;
    t.tm_mday = date->Date;
    t.tm_hour = time->Hours;
    t.tm_min  = time->Minutes;
    t.tm_sec  = time->Seconds;

    const uint32_t ms = rtc_subseconds_to_ms(time);

    return (utc_tm_to_unix_seconds(&t) * 1000ULL) + ms;
}

/// @brief Converts a unix epoch time in sec to a HAL RTC time.
/// @param unix_sec Unix timestamp in seconds.
/// @param rtc_time Destination for HAL RTC time.
/// @param rtc_date Destination for HAL RTC date.
/// @note It is not possible to write a ms version of this function,
//        as the RTC subseconds are always set to 0 when the time is synced.
void TIME_unix_epoch_time_sec_to_rtc_hal(
    uint64_t unix_sec,
    RTC_TimeTypeDef *rtc_time,
    RTC_DateTypeDef *rtc_date
) {
    // Clear the structs.
    memset(rtc_time, 0, sizeof(RTC_TimeTypeDef));
    memset(rtc_date, 0, sizeof(RTC_DateTypeDef));

    const time_t unix_sec_time_t = unix_sec;
    struct tm tm_time;
    gmtime_r(&unix_sec_time_t, &tm_time);

    // Time.
    rtc_time->Hours   = tm_time.tm_hour;
    rtc_time->Minutes = tm_time.tm_min;
    rtc_time->Seconds = tm_time.tm_sec;

    rtc_time->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    rtc_time->StoreOperation = RTC_STOREOPERATION_RESET;

    // Subseconds are always set to 0 when the time is synced.
    // Cannot set to anything else here.

    // Date.
    rtc_date->Year  = tm_time.tm_year - 100; // tm_year = years since 1900 -> RTC expects since 2000.
    rtc_date->Month = tm_time.tm_mon + 1;    // tm_mon = 0-11
    rtc_date->Date  = tm_time.tm_mday;

    /* tm_wday = 0–6 (Sun–Sat), RTC = 1–7 */
    rtc_date->WeekDay = tm_time.tm_wday + 1;
}
