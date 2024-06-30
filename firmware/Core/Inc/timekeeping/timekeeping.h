#ifndef __INCLUDE_GUARD__TIMEKEEPING_H_
#define __INCLUDE_GUARD__TIMEKEEPING_H_

#include <stdint.h>
#include <stdlib.h>

#define TIM_EPOCH_DECIMAL_STRING_LEN 14

#define TIM_RTC_STARTUP_DELAY_TICKS 200

typedef enum TIM_SYNC_SOURCE {
    TIM_SOURCE_NONE = 0,
    TIM_SOURCE_GNSS,
    TIM_SOURCE_RTC,
    TIM_SOURCE_TELECOMMAND,
} TIM_sync_source_t;

uint32_t TIM_get_current_system_uptime_ms(void);
void TIM_set_current_unix_epoch_time_ms(uint64_t current_unix_epoch_time_ms, TIM_sync_source_t source);
uint64_t TIM_get_current_unix_epoch_time_ms();

void TIM_get_timestamp_string(char *log_str, size_t max_len); 
void TIM_get_timestamp_string_datetime(char *log_str, size_t max_len);
char TIM_synchronization_source_letter(TIM_sync_source_t source);
void TIM_epoch_ms_to_decimal_string(char *str, size_t len);

void TIM_Init(void);
uint64_t TIM_get_rtc_unix_epoch_ms(void);
void TIM_set_rtc_from_unix_epoch_ms(uint64_t ms);

#endif // __INCLUDE_GUARD__TIMEKEEPING_H_
