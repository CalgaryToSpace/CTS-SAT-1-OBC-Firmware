#include "log/log.h"
#include "log/log_sinks.h"
#include "debug_tools/debug_uart.h"
#include "timekeeping/timekeeping.h"

#include <complex.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdarg.h>

// Inspired by uLog: https://github.com/rdpoor/ulog

// Internal interfaces and variables

#define LOG_TIMESTAMP_MAX_LENGTH 30
#define LOG_CHANNEL_NAME_MAX_LENGTH 20
#define LOG_SYSTEM_NAME_MAX_LENGTH 20
// Messages up to 256 characters
#define LOG_FORMATTED_MESSAGE_MAX_LENGTH 256
// Includes prefix, with cushion for delimiters and newline
#define LOG_FULL_MESSAGE_MAX_LENGTH ( LOG_FORMATTED_MESSAGE_MAX_LENGTH + LOG_TIMESTAMP_MAX_LENGTH + LOG_CHANNEL_NAME_MAX_LENGTH + LOG_SYSTEM_NAME_MAX_LENGTH + 1 )

static char LOG_timestamp_string[LOG_TIMESTAMP_MAX_LENGTH] = {0};
static char LOG_formatted_log_message[LOG_FORMATTED_MESSAGE_MAX_LENGTH] = {0};
static char LOG_full_log_message[LOG_FULL_MESSAGE_MAX_LENGTH] = {0};

typedef struct {
    LOG_channel_enum_t channel;
    const char name[LOG_CHANNEL_NAME_MAX_LENGTH];
    uint8_t enabled;
    uint32_t severity_mask;
} LOG_channel_t;

typedef struct {
    LOG_system_enum_t system;
    char name[LOG_SYSTEM_NAME_MAX_LENGTH];
    char *log_file_path;
    uint8_t logging_enabled;
    uint32_t severity_mask;
} LOG_system_t;

static const uint8_t LOG_SEVERITY_MASK_ALL = 0xFF;
// No debugging messages by default
static const uint8_t LOG_SEVERITY_MASK_DEFAULT = LOG_SEVERITY_MASK_ALL & ~(uint8_t)LOG_SEVERITY_DEBUG;

static LOG_channel_t LOG_channels[] = {
    {LOG_CHANNEL_FILE, "log files", LOG_CHANNEL_OFF, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_CHANNEL_UHF_RADIO, "UHF radio", LOG_CHANNEL_OFF, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_CHANNEL_UMBILICAL_UART, "umbilical UART", LOG_CHANNEL_ON, LOG_SEVERITY_MASK_DEFAULT},
};
static const uint16_t LOG_NUMBER_OF_CHANNELS = sizeof(LOG_channels) / sizeof(LOG_channel_t);

static LOG_system_t LOG_systems[] = {
    {LOG_SYSTEM_OBC, "OBC", "/logs/obc_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_UHF_RADIO, "UHF_RADIO", "/logs/uhf_radio_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_UMBILICAL_UART, "UMBILICAL_UART", "/logs/umbilical_uart_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_GPS, "GPS", "/logs/gps_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_MPI, "MPI", "/logs/mpi_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_EPS, "EPS", "/logs/eps_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_BOOM, "BOOM", "/logs/boom_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_ADCS, "ADCS", "/logs/adcs_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_LFS, "LFS", "/logs/lfs_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_FLASH, "FLASH", "/logs/flash_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_ANTENNA_DEPLOY, "ANTENNA_DEPLOY", "/logs/antenna_deploy_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_LOG, "LOG", "/logs/log_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_TELECOMMAND, "TELECOMMAND", "/logs/telecommand_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_UNIT_TEST, "UNIT_TEST", "/logs/unit_test_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
// LOG_SYSTEM_UNKNOWN must be the LAST entry so it is easy to find below
    {LOG_SYSTEM_UNKNOWN, "UNKNOWN", "/logs/unknown_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
};
static const uint16_t LOG_NUMBER_OF_SYSTEMS = sizeof(LOG_systems) / sizeof(LOG_system_t);

// External interfaces 

/// @brief Log a message to several communication channels
/// @param source source of log message (i.e., satellite subsystem)
/// @param channels bitwise-OR'd destination channels
/// @param severity message severity
/// @param msg message text
/// @return void
/// @details Normally msg does not end with a newline (\n).
///     Exclude one or more channels using LOG_all_channels_except(...)
void LOG_message(LOG_system_enum_t source, LOG_severity_enum_t severity, uint32_t channel_mask, const char fmt[], ...)
{
    // Get the system time
    TIM_get_timestamp_string(LOG_timestamp_string, LOG_TIMESTAMP_MAX_LENGTH);

    const char *severity_text = LOG_get_severity_name(severity);

    // Prepare the message according to the requested format
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(LOG_formatted_log_message, LOG_FORMATTED_MESSAGE_MAX_LENGTH, fmt, ap);
    va_end(ap);

    // Prepare the full message including time and severity
    // Defaults to "UNKNOWN"
    LOG_system_t *system = &LOG_systems[LOG_NUMBER_OF_SYSTEMS - 1];
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system == source) {
            system = &LOG_systems[i];
            break;
        }
    }
    snprintf(LOG_full_log_message, LOG_FULL_MESSAGE_MAX_LENGTH, 
            "%s [%s:%s]: %s\n", 
            LOG_timestamp_string, 
            system->name, 
            severity_text, 
            LOG_formatted_log_message
    );

    // Send message to enabled channels
    LOG_channel_t *c;
    for (uint16_t i = 0; i < LOG_NUMBER_OF_CHANNELS; i++) {
        c = &LOG_channels[i];
        if (c->enabled && (c->channel & channel_mask) && (severity & c->severity_mask) && (severity & system->severity_mask)) {
            switch (c->channel) {
                case LOG_CHANNEL_FILE:
                    // Send to log file if subsystem logging is enabled
                    if (system->logging_enabled) {
                        LOG_to_file(system->log_file_path, LOG_full_log_message);
                    }
                    break;
                case LOG_CHANNEL_UHF_RADIO:
                    LOG_to_uhf_radio(LOG_full_log_message);
                    break;
                case LOG_CHANNEL_UMBILICAL_UART:
                    LOG_to_umbilical_uart(LOG_full_log_message);
                    break;
                default:
                    // TODO: Is recursion allowed?
                    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_CRITICAL, 
                            LOG_CHANNEL_ALL, "Error: unknown log channel %s", 
                            c->name
                    );
                    break;
            }
        }
    }
    
    return;
}

/// @brief Returns all channels, except the specified exceptions
/// @param exceptions Bitfield represent log channels to exclude
/// @return Bitfield representing all log channels, except those passed as an arg
/// @note Especially useful in calls to `LOG_message()`, to report a failure of a specific logging sink.
uint32_t LOG_all_channels_except(uint32_t channel_exceptions_mask)
{
    return LOG_CHANNEL_ALL & ~channel_exceptions_mask;
}

/// @brief Check logging channel status
/// @param channel the channel to check
/// @return 0: disabled (or invalid channel); 1: enabled
uint8_t LOG_is_channel_enabled(LOG_channel_enum_t channel)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_CHANNELS; i++) {
        if (LOG_channels[i].channel == channel) {
            return LOG_channels[i].enabled;
        }
    }
    
    // Channel not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LOG_is_channel_enabled(): unknown channel: %d", channel);
    
    return 0;
}

/// @brief Turn debugging messages on or off for the specified channel
/// @param channel specified channel 
/// @param state 0: debugging messages off; 1: debugging messages on
/// @details Turning off a channel's debugging messages overrides a system's
///     debugging messages state
void LOG_set_channel_debugging_messages_state(LOG_channel_enum_t channel, uint8_t state)
{
    uint32_t mask = LOG_SEVERITY_DEBUG;
    for (uint16_t i = 0; i < LOG_NUMBER_OF_CHANNELS; i++) {
        if (LOG_channels[i].channel == channel) {
            if (state) {
                LOG_channels[i].severity_mask |= mask;
                LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "Enabled debugging on the %s channel", LOG_channels[i].name);
            }
            else {
                LOG_channels[i].severity_mask &= ~mask;
                LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "Disabled debugging on the %s channel", LOG_channels[i].name);
            }
            return;
        }
    }

    // Channel not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LOG_set_channel_debugging_messages_state(): unknown channel: %d", channel);

    return;
}

/// @brief Turn debugging messages on or off for the specified system
/// @param system specified system
/// @param state 0: debugging messages off; 1: debugging messages on
/// @details Debugging messages will be disabled if turned off for a given
///     channel
void LOG_set_system_debugging_messages_state(LOG_system_enum_t system, uint8_t state)
{
    uint32_t mask = LOG_SEVERITY_DEBUG;
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system == system) {
            if (state) {
                LOG_systems[i].severity_mask |= mask;
                LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "Enabled debugging for the %s system", LOG_systems[i].name);
            }
            else {
                LOG_systems[i].severity_mask &= ~mask;
                LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "Disabled debugging for the %s system", LOG_systems[i].name);
            }
            return;
        }
    }

    // System not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LOG_set_system_debugging_messages_state(): unknown system: %d", system);

    return;
}

/// @brief Check whether file logging is enabled for a subsystem
/// @param system
/// @return returns 1 if logging to file is enabled, 0 otherwise.
uint8_t LOG_is_file_logging_enabled_for_system(LOG_system_enum_t system)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system == system) {
            return LOG_systems[i].logging_enabled;
        }
    }
    
    // System not found? Return false
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LOG_is_file_logging_enabled_for_system(): unknown system: %d", system);
    return 0;
}

/// @brief Set the logging severity bitfield mask for a subsystem
/// @param system specified system
/// @param severity_mask bitfield representing the severities to be logged
void LOG_set_severity_mask_for_system(LOG_system_enum_t system, uint32_t severity_mask)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system == system) {
            LOG_systems[i].severity_mask = severity_mask;
            return;
        }
    }

    // System not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LOG_set_severity_mask_for_system(): unknown system: %d", system);
    return;
}

/// @brief Enable or disable logging to file for a subsystem
/// @param system specified subsystem
/// @state 0: disable  1:  enable
void LOG_set_file_logging_state_for_system(LOG_system_enum_t system, uint8_t state)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system == system) {
            LOG_systems[i].logging_enabled = state;
            LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, 
                    "%s file logging for %s", 
                    state == 0 ? "Disabled" : "Enabled", 
                    LOG_systems[i].name
            );
            return;
        }
    }

    // System not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LOG_set_file_logging_state_for_system(): unknown system: %d", system);
    return;
}

/// @brief Enable or disable logging to a channel
/// @param channel specified channel
/// @state 0: disable  1:  enable
void LOG_set_channel_state(LOG_channel_enum_t channel, uint8_t state)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_CHANNELS; i++) {
        if (LOG_channels[i].channel == channel) {
            LOG_channels[i].enabled = state;
            LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, 
                    "%s logging to %s", 
                    state == 0 ? "Disabled" : "Enabled", 
                    LOG_channels[i].name
            );
            return;
        }
    }

    // Channel not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LOG_set_channel_state(): unknown system: %d", system);
    return;
}

/// @brief Report a channel's logging state
/// @param channel specified channel
void LOG_report_channel_logging_state(LOG_channel_enum_t channel)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_CHANNELS; i++) {
        if (LOG_channels[i].channel == channel) {
            LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "%20s: %s",  LOG_channels[i].name, LOG_channels[i].enabled ? "enabled" : "disabled");
            return;
        }
    }

    // Channel not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LOG_report_channel_status(): unknown channel: %d", channel);

    return;
}

/// @brief Report a subsystem's file-logging state
/// @param channel specified channel
/// @note includes the subsystem's log filename
void LOG_report_system_file_logging_state(LOG_system_enum_t system)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system == system) {
            LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "%20s: %9s (log file: '%s')",  LOG_systems[i].name, LOG_systems[i].logging_enabled ? "enabled" : "disabled", LOG_systems[i].log_file_path);
            return;
        }
    }

    // System not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LOG_report_channel_status(): unknown system: %d", system);
    return;
}

uint16_t LOG_number_of_logging_channels(void)
{
    return LOG_NUMBER_OF_CHANNELS;
}

uint16_t LOG_number_of_logging_systems(void)
{
    return LOG_NUMBER_OF_SYSTEMS;
}

/// @brief Get the name of a severity enum
/// @param severity the specified severity
/// @return pointer to the statically allocated name
const char* LOG_get_severity_name(LOG_severity_enum_t severity) 
{
    switch (severity) {
        case LOG_SEVERITY_DEBUG:
            return "DEBUG";
        case LOG_SEVERITY_NORMAL:
            return "NORMAL";
        case LOG_SEVERITY_WARNING:
            return "WARNING";
        case LOG_SEVERITY_ERROR:
            return "ERROR";
        case LOG_SEVERITY_CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN SEVERITY";
    }
}

