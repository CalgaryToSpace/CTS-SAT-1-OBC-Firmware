#ifndef __INCLUDE__GUARD__LOG_H_
#define __INCLUDE__GUARD__LOG_H_

#include <stdint.h>

// Restricted by radio packet length
#define LOG_FORMATTED_MESSAGE_MAX_LENGTH 185

typedef enum {
    LOG_SEVERITY_DEBUG = 1 << 0,
    LOG_SEVERITY_NORMAL = 1 << 1,
    LOG_SEVERITY_WARNING = 1 << 2,
    LOG_SEVERITY_ERROR = 1 << 3,
    LOG_SEVERITY_CRITICAL = 1 << 4,
} LOG_severity_enum_t;

typedef enum {
    LOG_SINK_NONE = 0,
    LOG_SINK_UHF_RADIO = 1 << 0,
    LOG_SINK_FILE = 1 << 1,
    LOG_SINK_UMBILICAL_UART = 1 << 2,
    LOG_SINK_UNKNOWN = 1 << 3,
    LOG_SINK_ALL = (1 << 4) - 1,
} LOG_sink_enum_t;

enum {
    LOG_SINK_OFF = 0,
    LOG_SINK_ON = 1,
};

typedef enum {
    LOG_SYSTEM_OBC = 1 << 0,
    LOG_SYSTEM_UHF_RADIO = 1 << 1,
    LOG_SYSTEM_UMBILICAL_UART = 1 << 2,
    LOG_SYSTEM_GPS = 1 << 3,
    LOG_SYSTEM_MPI = 1 << 4,
    LOG_SYSTEM_EPS = 1 << 5,
    LOG_SYSTEM_BOOM = 1 << 6,
    LOG_SYSTEM_ADCS = 1 << 7,
    LOG_SYSTEM_LFS = 1 << 8,
    LOG_SYSTEM_FLASH = 1 << 9,
    LOG_SYSTEM_ANTENNA_DEPLOY = 1 << 10,
    LOG_SYSTEM_LOG = 1 << 11,
    LOG_SYSTEM_TELECOMMAND = 1 << 12,
    LOG_SYSTEM_UNIT_TEST = 1 << 13,
    LOG_SYSTEM_UNKNOWN = 1 << 14,
    LOG_SYSTEM_ALL = (1 << 15) - 1,
} LOG_system_enum_t;

enum {
    LOG_SYSTEM_OFF = 0,
    LOG_SYSTEM_ON = 1,
};

void LOG_message(
    LOG_system_enum_t source, LOG_severity_enum_t severity, uint32_t sink_mask,
    const char fmt[], ...
) __attribute__((format(printf, 4, 5)));
// __attribute__ part indicates that the `fmt` arg (4th arg) is a printf format string, and the
// 5th arg is a variable argument list. Makes compiler check that fmt matches the variable arguments.

uint32_t LOG_all_sinks_except(uint32_t exceptions);
uint8_t LOG_is_sink_enabled(LOG_sink_enum_t sink);
void LOG_set_sink_debugging_messages_enabled_state(LOG_sink_enum_t sink, uint8_t state);
void LOG_set_system_debugging_messages_enabled_state(LOG_system_enum_t system, uint8_t state);
uint8_t LOG_is_system_file_logging_enabled(LOG_system_enum_t system);
void LOG_set_system_severity_mask(LOG_system_enum_t system, uint32_t severity_mask);
void LOG_set_sink_enabled_state(LOG_sink_enum_t sink, uint8_t state);
void LOG_set_system_file_logging_enabled_state(LOG_system_enum_t system, uint8_t state);
void LOG_report_sink_enabled_state(LOG_sink_enum_t sinks);
void LOG_report_system_file_logging_state(LOG_system_enum_t systems);
uint16_t LOG_number_of_logging_sinks(void);
uint16_t LOG_number_of_logging_systems(void);
const char* LOG_get_severity_name(LOG_severity_enum_t severity);
uint8_t LOG_memory_table_max_entries(void);
uint8_t LOG_get_memory_table_index_of_most_recent_log_entry(void);
const char *LOG_get_memory_table_full_message_at_index(uint8_t index);
const char *LOG_get_most_recent_log_message_text(void);

#endif // __INCLUDE__GUARD__LOG_H_
