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

static const uint8_t days_in_month[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};


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


void TIME_unix_epoch_time_ms_to_rtc_hal(
    uint64_t unix_ms,
    RTC_TimeTypeDef *time,
    RTC_DateTypeDef *date
) {
    uint64_t unix_sec = unix_ms / 1000;
    uint32_t ms = unix_ms % 1000;

    uint32_t days = unix_sec / SECONDS_PER_DAY;
    uint32_t secs_of_day = unix_sec % SECONDS_PER_DAY;

    time->Hours   = secs_of_day / 3600;
    secs_of_day  %= 3600;
    time->Minutes = secs_of_day / 60;
    time->Seconds = secs_of_day % 60;

    time->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    time->StoreOperation = RTC_STOREOPERATION_RESET;

    /* Convert ms → subseconds */
    const uint32_t second_fraction = hrtc.Init.SynchPrediv;
    time->SubSeconds = ms_to_rtc_subseconds(ms, second_fraction);

    /* Date calculation */
    uint16_t year = 1970;

    while (1) {
        const uint16_t days_in_year = is_leap_year(year) ? 366 : 365;
        if (days < days_in_year) break;
        days -= days_in_year;
        year++;
    }

    uint8_t month = 0;
    while (1) {
        uint8_t dim = days_in_month[month];
        if (month == 1 && is_leap_year(year)) dim++;

        if (days < dim) break;
        days -= dim;
        month++;
    }

    date->Year  = year - 2000;
    date->Month = month + 1;
    date->Date  = days + 1;

    date->WeekDay = ((unix_sec / SECONDS_PER_DAY) + 4) % 7 + 1;
}
