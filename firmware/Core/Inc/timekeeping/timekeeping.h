#ifndef INCLUDE_GUARD__TIMEKEEPING_H_
#define INCLUDE_GUARD__TIMEKEEPING_H_

#include <stdint.h>
#include <stdlib.h>

#define TIM_EPOCH_DECIMAL_STRING_LEN 13

typedef enum TIM_SYNC_SOURCE {
    TIM_SOURCE_NONE = 0,
    TIM_SOURCE_GNSS,
    TIM_SOURCE_TELECOMMAND_ABSOLUTE,
    TIM_SOURCE_TELECOMMAND_CORRECTION,
} TIM_sync_source_t;

uint32_t TIM_get_current_system_uptime_ms(void);
void TIM_set_current_unix_epoch_time_ms(uint64_t current_unix_epoch_time_ms, TIM_sync_source_t source);
uint64_t TIM_get_current_unix_epoch_time_ms();

void TIM_get_timestamp_string(char *log_str, size_t max_len); 
void TIM_get_timestamp_string_datetime(char *log_str, size_t max_len);
char TIME_sync_source_enum_to_letter_char(TIM_sync_source_t source);

extern uint64_t TIM_unix_epoch_time_at_last_time_resync_ms;
extern uint32_t TIM_system_uptime_at_last_time_resync_ms;
extern TIM_sync_source_t TIM_last_synchronization_source;

#endif // INCLUDE_GUARD__TIMEKEEPING_H_
