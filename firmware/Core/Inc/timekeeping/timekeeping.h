#ifndef __INCLUDE_GUARD__TIMEKEEPING_H_
#define __INCLUDE_GUARD__TIMEKEEPING_H_

#include <stdint.h>
#include <stdlib.h>

#define TIM_EPOCH_DECIMAL_STRING_LEN 14

typedef enum TIM_SYNC_SOURCE {
    TIM_SOURCE_NONE = 0,
    TIM_SOURCE_GNSS,
    TIM_SOURCE_TELECOMMAND,
} TIM_sync_source_t;

uint32_t TIM_get_current_system_uptime_ms(void);
void TIM_set_current_unix_epoch_time_ms(uint64_t current_unix_epoch_time_ms, TIM_sync_source_t source);
uint64_t TIM_get_current_unix_epoch_time_ms();

void TIM_get_timestamp_string(char *log_str, size_t max_len); 
void TIM_get_timestamp_string_datetime(char *log_str, size_t max_len);
char TIM_synchronization_source_letter(TIM_sync_source_t source);
void TIM_epoch_ms_to_decimal_string(char *str, size_t len);

extern uint64_t TIM_unix_epoch_time_at_last_time_resync_ms;
extern uint32_t TIM_system_uptime_at_last_time_resync_ms;
extern TIM_sync_source_t TIM_last_synchronization_source;

#endif // __INCLUDE_GUARD__TIMEKEEPING_H_
