#ifndef __INCLUDE__GUARD__LOG_H_
#define __INCLUDE__GUARD__LOG_H_

#include <stdint.h>

typedef enum {
    LOG_SEVERITY_DEBUG = 1 << 0,
    LOG_SEVERITY_NORMAL = 1 << 1,
    LOG_SEVERITY_WARNING = 1 << 2,
    LOG_SEVERITY_ERROR = 1 << 3,
    LOG_SEVERITY_CRITICAL = 1 << 4,
} LOG_severity_enum_t;

typedef enum {
    LOG_CHANNEL_NONE = 0,
    LOG_CHANNEL_UHF_RADIO = 1 << 0,
    LOG_CHANNEL_FILE = 1 << 1,
    LOG_CHANNEL_UMBILICAL_UART = 1 << 2,
    LOG_CHANNEL_UNKNOWN = 1 << 3,
    LOG_CHANNEL_ALL = (1 << 4) - 1,
} LOG_channel_enum_t;

enum {
    LOG_CHANNEL_OFF = 0,
    LOG_CHANNEL_ON = 1,
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

void LOG_message(LOG_system_enum_t source, LOG_severity_enum_t severity, uint32_t channel_mask, const char fmt[], ...);
uint32_t LOG_all_channels_except(uint32_t exceptions);
uint8_t LOG_is_channel_enabled(LOG_channel_enum_t channel);
void LOG_set_channel_debugging_messages_state(LOG_channel_enum_t channel, uint8_t state);
void LOG_set_system_debugging_messages_state(LOG_system_enum_t system, uint8_t state);
uint8_t LOG_is_system_file_logging_enabled(LOG_system_enum_t system);
void LOG_set_system_severity_mask(LOG_system_enum_t system, uint32_t severity_mask);
void LOG_set_system_file_logging_state(LOG_system_enum_t system, uint8_t state);
void LOG_set_channel_state(LOG_channel_enum_t channel, uint8_t state);
void LOG_report_channel_logging_state(LOG_channel_enum_t channels);
void LOG_report_system_file_logging_state(LOG_system_enum_t systems);
uint16_t LOG_number_of_logging_channels(void);
uint16_t LOG_number_of_logging_systems(void);
const char* LOG_get_severity_name(LOG_severity_enum_t severity);

#endif // __INCLUDE__GUARD__LOG_H_
