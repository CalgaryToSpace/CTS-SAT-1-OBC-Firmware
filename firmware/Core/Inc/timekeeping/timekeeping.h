#ifndef INCLUDE_GUARD__TIMEKEEPING_H_
#define INCLUDE_GUARD__TIMEKEEPING_H_

#include <stdint.h>
#include <stdlib.h>

#define TIME_EPOCH_DECIMAL_STRING_LEN 13

typedef enum {
    TIME_SYNC_SOURCE_NONE = 0,
    TIME_SYNC_SOURCE_GNSS_UART,
    TIME_SYNC_SOURCE_GNSS_PPS,
    TIME_SYNC_SOURCE_TELECOMMAND_ABSOLUTE,
    TIME_SYNC_SOURCE_TELECOMMAND_CORRECTION,
    TIME_SYNC_SOURCE_EPS_RTC // Electrical Power System's Real-Time Clock (precise to 1 second only)
} TIME_sync_source_enum_t;

uint32_t TIME_get_current_system_uptime_ms(void);
void TIME_set_current_unix_epoch_time_ms(uint64_t current_unix_epoch_time_ms, TIME_sync_source_enum_t source);
uint64_t TIME_get_current_unix_epoch_time_ms();
uint64_t TIME_convert_uptime_to_unix_epoch_time_ms(uint32_t uptime_ms);

void TIME_format_timestamp_str(
    char dest_str[], size_t dest_str_size,
    uint32_t uptime_ms, TIME_sync_source_enum_t sync_source
);
void TIME_get_current_timestamp_str(char *dest_str, size_t dest_str_size); 

void TIME_format_utc_datetime_str(
    char *dest_str, size_t dest_str_size,
    uint64_t timestamp_ms, TIME_sync_source_enum_t sync_source
);
void TIME_get_current_utc_datetime_str(char *dest_str, size_t dest_str_size);

char TIME_sync_source_enum_to_letter_char(TIME_sync_source_enum_t source);

extern uint64_t TIME_unix_epoch_time_at_last_time_resync_ms;
extern uint32_t TIME_system_uptime_at_last_time_resync_ms;
extern TIME_sync_source_enum_t TIME_last_synchronization_source;

#endif // INCLUDE_GUARD__TIMEKEEPING_H_
