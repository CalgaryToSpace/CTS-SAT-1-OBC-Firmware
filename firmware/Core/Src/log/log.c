#include "log/log.h"
#include "debug_tools/debug_uart.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "timekeeping/timekeeping.h"

#include <complex.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdarg.h>

// Inspired by uLog: https://github.com/rdpoor/ulog

extern UART_HandleTypeDef hlpuart1;
extern uint8_t LFS_is_lfs_mounted;

// Internal interfaces and variables

#define LOG_TIMESTAMP_MAX_LENGTH 30
#define LOG_CHANNEL_NAME_MAX_LENGTH 20
#define LOG_SYSTEM_NAME_MAX_LENGTH 20
// Messages up to 256 characters
#define LOG_FORMATTED_MESSAGE_MAX_LENGTH 256
// Includes prefix, with cushion for delimiters and newline
#define LOG_FULL_MESSAGE_MAX_LENGTH ( LOG_FORMATTED_MESSAGE_MAX_LENGTH + LOG_TIMESTAMP_MAX_LENGTH + LOG_CHANNEL_NAME_MAX_LENGTH + LOG_SYSTEM_NAME_MAX_LENGTH + 1 )
#define LOG_UART_TRANSMIT_TIMEOUT 200

static char LOG_timestamp_string[LOG_TIMESTAMP_MAX_LENGTH] = {0};
static char LOG_formatted_log_message[LOG_FORMATTED_MESSAGE_MAX_LENGTH] = {0};
static char LOG_full_log_message[LOG_FULL_MESSAGE_MAX_LENGTH] = {0};

typedef struct {
    LOG_channel_enum_t channel;
    const char name[LOG_CHANNEL_NAME_MAX_LENGTH];
    uint8_t enabled;
    uint8_t severity_mask;
} LOG_channel_t;

typedef struct {
    LOG_system_enum_t system;
    char name[LOG_SYSTEM_NAME_MAX_LENGTH];
    char *log_file_path;
    uint8_t logging_enabled;
    uint8_t severity_mask;
} LOG_system_t;

void LOG_to_file(const char filename[], const char msg[]);
void LOG_to_umbilical_uart(const char msg[]);
void LOG_to_uhf_radio(const char msg[]);
void LOG_set_channel_state(LOG_channel_enum_t channel, uint8_t state);
void LOG_set_system_file_logging_state(LOG_system_enum_t systems, uint8_t state);

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
/// @param from source of log message (i.e., satellite subsystem)
/// @param channels bitwise-OR'd destination channels
/// @param severity message severity
/// @param msg message text
/// @return void
/// @details Normally msg does not end with a newline (\n).
///     Exclude one or more channels using LOG_all_channels_except(...)
void LOG_message(LOG_system_enum_t from, LOG_severity_enum_t severity, LOG_channel_enum_t channels, const char fmt[], ...)
{
    // Get the system time
    TIM_get_timestamp_string(LOG_timestamp_string, LOG_TIMESTAMP_MAX_LENGTH);

    const char *severity_text = LOG_severity_enum_t_to_str(severity);

    // Prepare the message according to the requested format
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(LOG_formatted_log_message, LOG_FORMATTED_MESSAGE_MAX_LENGTH, fmt, ap);
    va_end(ap);

    // Prepare the full message including time and severity
    // Defaults to "UNKNOWN"
    LOG_system_t *system = &LOG_systems[LOG_NUMBER_OF_SYSTEMS - 1];
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system == from) {
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
        if (c->enabled && (c->channel & channels) && (c->severity_mask & severity) && (system->severity_mask & severity)) {
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
LOG_channel_enum_t LOG_all_channels_except(LOG_channel_enum_t exceptions)
{
    return LOG_CHANNEL_ALL & ~exceptions;
}

/// @brief Enable the specified logging channel
/// @param channels Bitwise OR'd channels to enable
void LOG_enable_channels(LOG_channel_enum_t channels)
{
    LOG_set_channel_state(channels, 1);
    return;
}

/// @brief Disable the specified logging channel
/// @param channels Bitwise OR'd channels to disable 
void LOG_disable_channels(LOG_channel_enum_t channels)
{
    LOG_set_channel_state(channels, 0);
    return;
}

/// @brief Check logging channel status
/// @param channel the channel to check
/// @return 0: disabled (or invalid channel); 1: enabled
uint8_t LOG_channel_is_enabled(LOG_channel_enum_t channel)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_CHANNELS; i++) {
        if (LOG_channels[i].channel == channel) {
            return LOG_channels[i].enabled;
        }
    }
    
    // System not found
    return 0;
}

/// @brief Turn debugging messages on or off for the specified channels
/// @param channels bitwise-OR'd channels to adjust
/// @param state 0: debugging messages off; 1: debugging messages on
/// @details Turning off a channel's debugging messages overrides a system's
///     debugging messages state
void LOG_set_channel_debugging_messages_state(LOG_channel_enum_t channels, uint8_t state)
{
    uint8_t mask = LOG_SEVERITY_DEBUG;
    for (uint16_t i = 0; i < LOG_NUMBER_OF_CHANNELS; i++) {
        if (LOG_channels[i].channel & channels) {
            if (state) {
                LOG_channels[i].severity_mask |= mask;
                LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "Enabled debugging on the %s channel", LOG_channels[i].name);
            }
            else {
                LOG_channels[i].severity_mask &= ~mask;
                LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "Disabled debugging on the %s channel", LOG_channels[i].name);
            }
        }
    }

    return;
}

/// @brief Turn debugging messages on or off for the specified systems
/// @param systems bitwise-OR'd systems to adjust
/// @param state 0: debugging messages off; 1: debugging messages on
/// @details Debugging messages will be disabled if turned off for a given
///     channel
void LOG_set_system_debugging_messages_state(LOG_system_enum_t systems, uint8_t state)
{
    uint8_t mask = LOG_SEVERITY_DEBUG;
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system & systems) {
            if (state) {
                LOG_systems[i].severity_mask |= mask;
                LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "Enabled debugging for the %s system", LOG_systems[i].name);
            }
            else {
                LOG_systems[i].severity_mask &= ~mask;
                LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "Disabled debugging for the %s system", LOG_systems[i].name);
            }
        }
    }

    return;
}

/// @brief Enable file logging for the specified systems
/// @param systems Bitwise OR'd systems to enable
/// @return returns 0 on success
void LOG_enable_systems(LOG_system_enum_t systems)
{
    LOG_set_system_file_logging_state(systems, 1);
    return;
}

/// @brief Disable file logging for the specified systems
/// @param systems Bitwise OR'd systems to enable
/// @return returns 0 on success
void LOG_disable_systems(LOG_system_enum_t systems)
{
    LOG_set_system_file_logging_state(systems, 0);
    return;
}

/// @brief Check whether logging to file is enabled for a subsystem
/// @param system
/// @return returns 1 if logging to file is enabled, 0 otherwise.
uint8_t LOG_system_logging_is_enabled(LOG_system_enum_t system)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system == system) {
            return 1;
        }
    }
    
    // System not found? Return false
    return 0;
}

void LOG_set_severity_mask(LOG_system_enum_t systems, LOG_severity_enum_t severity_mask)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system & systems) {
            LOG_systems[i].severity_mask = severity_mask;
        }
    }

    return;
}

const char* LOG_severity_enum_t_to_str(LOG_severity_enum_t severity) 
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

void LOG_channels_status(LOG_channel_enum_t channels)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_CHANNELS; i++) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, 
                LOG_CHANNEL_ALL,
                "%20s: %s",  
                LOG_channels[i].name,
                LOG_channels[i].enabled ? "enabled" : "disabled"
        );
    }
}

void LOG_systems_status(LOG_system_enum_t systems)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, 
                LOG_CHANNEL_ALL,
                "%20s: %9s (log file: '%s')",  
                LOG_systems[i].name,
                LOG_systems[i].logging_enabled ? "enabled" : "disabled",
                LOG_systems[i].log_file_path
        );
    }
}


// Internal functions

/// @brief Sends a log message to a log file
/// @param filename full path of the log file
/// @param msg The message to be logged
/// @return void
void LOG_to_file(const char filename[], const char msg[])
{
    if (!LFS_is_lfs_mounted) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_CRITICAL, LOG_all_channels_except(LOG_CHANNEL_FILE), 
                "Error writing to system log file: LFS not mounted.\n");
        return;
    }

    // We cannot use LFS_append_file due to recursion of the logging system
    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, filename, LFS_O_RDWR | LFS_O_CREAT | LFS_O_APPEND, &LFS_file_cfg);
	if (open_result < 0)
	{
        // This error cannot be logged, except via UART or during an overpass 
        // of the ground station
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_CRITICAL, LOG_all_channels_except(LOG_CHANNEL_FILE), 
                "Error opening system log file.\n");
		return;
	}
    const lfs_soff_t offset = lfs_file_seek(&LFS_filesystem, &file, 0, LFS_SEEK_END);
    if (offset < 0) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_CRITICAL, LOG_all_channels_except(LOG_CHANNEL_FILE), 
                "Error seeking to end of system log file.\n");
        return;
    }

	const lfs_ssize_t bytes_written = lfs_file_write(&LFS_filesystem, &file, msg, strlen(msg));
	if (bytes_written < 0) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_CRITICAL, LOG_all_channels_except(LOG_CHANNEL_FILE), 
                "Error writing to system log file.\n");
		return;
	}
	
	// Close the File, the storage is not updated until the file is closed successfully
	const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
	if (close_result < 0) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_CRITICAL, LOG_all_channels_except(LOG_CHANNEL_FILE), 
                "Error closing system log file.\n");
		return;
	}
	
	return;
}

/// @brief Sends a log message to the umbilical UART
/// @param msg The message to be logged
/// @return void
void LOG_to_umbilical_uart(const char msg[]) 
{
    HAL_UART_Transmit(&hlpuart1, (uint8_t *)msg, strlen(msg),
                      LOG_UART_TRANSMIT_TIMEOUT);
    return;
}

/// @brief Sends a log message to the UHF radio
/// @param msg The message to be logged
/// @return void
void LOG_to_uhf_radio(const char msg[])
{
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_CRITICAL, LOG_CHANNEL_UMBILICAL_UART, "TODO: replace this with a UHF RADIO transmission");
    return;
}

/// @brief Enable or disable logging channels
/// @param channels Bitwise OR'd log channels
/// @state 0: disable  1:  enable
/// @return 0 on success
void LOG_set_channel_state(LOG_channel_enum_t channels, uint8_t state)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_CHANNELS; i++) {
        if (LOG_channels[i].channel & channels) {
            LOG_channels[i].enabled = state;
            LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, 
                    "%s logging via %s", 
                    state == 0 ? "Disabled" : "Enabled", 
                    LOG_channels[i].name
            );
        }
    }
    return;
}

/// @brief Enable or disable logging to file for specified systems
/// @param systems Bitwise OR'd system to toggle logging for
/// @state 0: disable  1:  enable
/// @return 0 on success
void LOG_set_system_file_logging_state(LOG_system_enum_t systems, uint8_t state)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system & systems) {
            LOG_systems[i].logging_enabled = state;
            LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, 
                    "%s file logging for %s", 
                    state == 0 ? "Disabled" : "Enabled", 
                    LOG_systems[i].name
            );
        }
    }
    return;
}

