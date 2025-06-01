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
#define LOG_SINK_NAME_MAX_LENGTH 20
#define LOG_SYSTEM_NAME_MAX_LENGTH 20

// Includes prefix, with cushion for delimiters, newline, and null terminator
#define LOG_FULL_MESSAGE_MAX_LENGTH ( LOG_FORMATTED_MESSAGE_MAX_LENGTH + LOG_TIMESTAMP_MAX_LENGTH + LOG_SINK_NAME_MAX_LENGTH + LOG_SYSTEM_NAME_MAX_LENGTH + 1 )

typedef struct {
    LOG_sink_enum_t sink;
    const char name[LOG_SINK_NAME_MAX_LENGTH];
    uint8_t enabled;
    uint32_t severity_mask;
} LOG_sink_t;

typedef struct {
    LOG_system_enum_t system;
    char name[LOG_SYSTEM_NAME_MAX_LENGTH];
    char *log_file_path;
    uint8_t file_logging_enabled;
    uint32_t severity_mask;
} LOG_system_t;

typedef struct {
    LOG_system_enum_t source;
    LOG_severity_enum_t severity;
    uint32_t sink_mask;
    char full_message[LOG_FULL_MESSAGE_MAX_LENGTH];
} LOG_memory_entry_t;


// Allocate memory buffers
static char LOG_timestamp_string[LOG_TIMESTAMP_MAX_LENGTH] = {0};
static char LOG_formatted_log_message[LOG_FORMATTED_MESSAGE_MAX_LENGTH] = {0};

// Memory sink is a circular buffer up to 256 entries
// Used for log backup in case of filesystem failure, and as independent
// buffer storage for preparing log messages, in case of recursive calls to
// LOG_message
#define LOG_MEMORY_NUMBER_OF_ENTRIES 128
static LOG_memory_entry_t LOG_memory_table[LOG_MEMORY_NUMBER_OF_ENTRIES] = {0};
// Start at last position in table, as first call to LOG_message will roll this to 0
static uint8_t LOG_memory_index_of_current_log_entry = LOG_MEMORY_NUMBER_OF_ENTRIES - 1;

// Severity masking.
static const uint8_t LOG_SEVERITY_MASK_ALL = 0xFF;
// TODO: Set mask to: No debugging messages by default.
// static const uint8_t LOG_SEVERITY_MASK_DEFAULT = LOG_SEVERITY_MASK_ALL & ~(uint8_t)LOG_SEVERITY_DEBUG;
static const uint8_t LOG_SEVERITY_MASK_DEFAULT = LOG_SEVERITY_MASK_ALL;

// Note: LOG_sinks entries must have same order as LOG_sink_enum_t entries.
// The in-memory log table is a combination of working memory for constructing
// log messages and a backup of the most recent log messages logged to at
// least one channel. It cannot be turned off. It is not included in the
// array of sinks.
static LOG_sink_t LOG_sinks[] = {
    {LOG_SINK_UHF_RADIO, "UHF radio", LOG_SINK_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SINK_FILE, "log files", LOG_SINK_OFF, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SINK_UMBILICAL_UART, "umbilical UART", LOG_SINK_ON, LOG_SEVERITY_MASK_DEFAULT},
};
static const uint16_t LOG_NUMBER_OF_SINKS = sizeof(LOG_sinks) / sizeof(LOG_sink_t);

// Note: LOG_systems entries must have same order as LOG_system_enum_t
// entries.
static LOG_system_t LOG_systems[] = {
    {LOG_SYSTEM_OBC, "OBC", "/logs/obc_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_UHF_RADIO, "UHF_RADIO", "/logs/uhf_radio_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_UMBILICAL_UART, "UMBILICAL_UART", "/logs/umbilical_uart_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_GNSS, "GNSS", "/logs/gnss_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
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
    // LOG_SYSTEM_UNKNOWN must be the LAST entry to make it easy to find below.
    {LOG_SYSTEM_UNKNOWN, "UNKNOWN", "/logs/unknown_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
};
static const uint16_t LOG_NUMBER_OF_SYSTEMS = sizeof(LOG_systems) / sizeof(LOG_system_t);

// External interfaces 

/// @brief Log a message to several destinations (sinks)
/// @param source source of log message (i.e., satellite subsystem)
/// @param severity message severity
/// @param sink_mask bitfield representing desired log sinks
/// @param fmt printf-link format
/// @param ... additional printf-like message arguments
/// @return void
/// @details Normally the message should not end with a newline (\n).
///     Exclude one or more sinks using LOG_all_sinks_except(...)
void LOG_message(LOG_system_enum_t source, LOG_severity_enum_t severity, uint32_t sink_mask, const char fmt[], ...)
{
    // Ensure quick return if debugging is disabled
    // Needed to maintain good hot-path performance
    if (severity == LOG_SEVERITY_DEBUG) {
        // Return early if debugging is not enabled for this system
        // Use __builtin_ctz to count trailing zeros in source to convert bitshifted enum to array index
        if (!(severity & LOG_systems[__builtin_ctz(source)].severity_mask)) { 
            return;
        }
        // Return early if debugging is not enabled for ALL of the requested sinks
        uint8_t debugging_enabled_for_at_least_one_sink = 0;
        for (uint16_t i = 0; i < LOG_NUMBER_OF_SINKS; i++) {
            if (LOG_sinks[i].enabled) {
                debugging_enabled_for_at_least_one_sink = 1;
                break;
            }
        }
        if (!debugging_enabled_for_at_least_one_sink) {
            return; 
        }
    }

    // Get the system time
    TIM_get_timestamp_string(LOG_timestamp_string, LOG_TIMESTAMP_MAX_LENGTH);

    // Get pointer to next storage slot in circular memory table
    LOG_memory_index_of_current_log_entry++;
    if (LOG_memory_index_of_current_log_entry > LOG_MEMORY_NUMBER_OF_ENTRIES - 1) {
        LOG_memory_index_of_current_log_entry = 0;
    }
    LOG_memory_entry_t *current_log_entry = &LOG_memory_table[LOG_memory_index_of_current_log_entry];
    current_log_entry->source = source;
    current_log_entry->severity = severity;
    current_log_entry->sink_mask = sink_mask;

    // Prepare the message according to the requested format
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(LOG_formatted_log_message, LOG_FORMATTED_MESSAGE_MAX_LENGTH, fmt, ap);
    va_end(ap);

    // Prepare the full message including time and severity
    // Defaults to "UNKNOWN" system
    const char *severity_text = LOG_get_severity_name(severity);
    LOG_system_t *system_config = &LOG_systems[LOG_NUMBER_OF_SYSTEMS - 1];
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system == source) {
            system_config = &LOG_systems[i];
            break;
        }
    }
    snprintf(current_log_entry->full_message, LOG_FULL_MESSAGE_MAX_LENGTH, 
            "%s [%s:%s]: %s\n", 
            LOG_timestamp_string, 
            system_config->name, 
            severity_text, 
            LOG_formatted_log_message
    );

    // Send message to each enabled sink.
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SINKS; i++) {
        const LOG_sink_t *sink_config = &LOG_sinks[i];
        if (
            sink_config->enabled
            && (sink_config->sink & sink_mask)
            && (severity & sink_config->severity_mask)
            && (severity & system_config->severity_mask)
        ) {
            switch (sink_config->sink) {
                case LOG_SINK_FILE:
                    // Send to log file if subsystem logging is enabled
                    if (system_config->file_logging_enabled) {
                        LOG_to_file(system_config->log_file_path, current_log_entry->full_message);
                    }
                    break;
                case LOG_SINK_UHF_RADIO:
                    LOG_to_uhf_radio(current_log_entry->full_message);
                    break;
                case LOG_SINK_UMBILICAL_UART:
                    LOG_to_umbilical_uart(current_log_entry->full_message);
                    break;
                case LOG_SINK_NONE:
                case LOG_SINK_UNKNOWN:
                default:
                    // Should not reach this except by memory corruption 
                    LOG_to_umbilical_uart("Error: unknown log sink\n");
                    break;
            }
        }
    }
    
    return;
}

/// @brief Returns all sinks, except the specified exceptions
/// @param exceptions Bitfield represent log sinks to exclude
/// @return Bitfield representing all log sinks, except those passed as an arg
/// @note Especially useful in calls to `LOG_message()`, to report a failure of a specific logging sink.
uint32_t LOG_all_sinks_except(uint32_t sink_exceptions_mask)
{
    return LOG_SINK_ALL & ~sink_exceptions_mask;
}

/// @brief Check logging sink status
/// @param sink the sink to check
/// @return 0: disabled (or invalid sink); 1: enabled
uint8_t LOG_is_sink_enabled(LOG_sink_enum_t sink)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SINKS; i++) {
        if (LOG_sinks[i].sink == sink) {
            return LOG_sinks[i].enabled;
        }
    }
    
    // Sink not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "LOG_is_sink_enabled(): unknown sink: %d", sink);
    
    return 0;
}

/// @brief Turn debugging messages on or off for the specified sink
/// @param sink specified sink 
/// @param state 0: debugging messages off; 1: debugging messages on
/// @details Turning off a sink's debugging messages overrides a system's
///     debugging messages state
void LOG_set_sink_debugging_messages_enabled_state(LOG_sink_enum_t sink, uint8_t state)
{
    const uint32_t mask = LOG_SEVERITY_DEBUG;
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SINKS; i++) {
        if (LOG_sinks[i].sink == sink) {
            if (state) {
                LOG_sinks[i].severity_mask |= mask;
                LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_SINK_ALL, "Enabled debugging on the %s sink", LOG_sinks[i].name);
            }
            else {
                LOG_sinks[i].severity_mask &= ~mask;
                LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_SINK_ALL, "Disabled debugging on the %s sink", LOG_sinks[i].name);
            }
            return;
        }
    }

    // Sink not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "LOG_set_sink_debugging_messages_enabled_state(): unknown sink: %d", sink);

    return;
}

/// @brief Turn debugging messages on or off for the specified system
/// @param system specified system
/// @param state 0: debugging messages off; 1: debugging messages on
/// @details Debugging messages will be disabled if turned off for a given
///     sink
void LOG_set_system_debugging_messages_enabled_state(LOG_system_enum_t system, uint8_t state)
{
    const uint32_t mask = LOG_SEVERITY_DEBUG;
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system == system) {
            if (state) {
                LOG_systems[i].severity_mask |= mask;
                LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_SINK_ALL, "Enabled debugging for the %s system", LOG_systems[i].name);
            }
            else {
                LOG_systems[i].severity_mask &= ~mask;
                LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_SINK_ALL, "Disabled debugging for the %s system", LOG_systems[i].name);
            }
            return;
        }
    }

    // System not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "LOG_set_system_debugging_messages_enabled_state(): unknown system: %d", system);

    return;
}

/// @brief Check whether file logging is enabled for a subsystem
/// @param system
/// @return returns 1 if logging to file is enabled, 0 otherwise.
uint8_t LOG_is_system_file_logging_enabled(LOG_system_enum_t system)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system == system) {
            return LOG_systems[i].file_logging_enabled;
        }
    }
    
    // System not found? Return false
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "LOG_is_system_file_logging_enabled(): unknown system: %d", system);
    return 0;
}

/// @brief Set the logging severity bitfield mask for a subsystem
/// @param system specified system
/// @param severity_mask bitfield representing the severities to be logged
void LOG_set_system_severity_mask(LOG_system_enum_t system, uint32_t severity_mask)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system == system) {
            LOG_systems[i].severity_mask = severity_mask;
            return;
        }
    }

    // System not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "LOG_set_system_severity_mask(): unknown system: %d", system);
    return;
}

/// @brief Enable or disable logging to a sink
/// @param sink specified sink
/// @state 0: disable  1:  enable
void LOG_set_sink_enabled_state(LOG_sink_enum_t sink, uint8_t state)
{
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SINKS; i++) {
        if (LOG_sinks[i].sink == sink) {
            LOG_sinks[i].enabled = state;
            LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_SINK_ALL, 
                    "%s logging to %s", 
                    state == 0 ? "Disabled" : "Enabled", 
                    LOG_sinks[i].name
            );
            return;
        }
    }

    // Sink not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "LOG_set_sink_state(): unknown sink: %d", sink);
    return;
}

/// @brief Enable or disable logging to file for a subsystem
/// @param system specified subsystem
/// @state 0: disable  1:  enable
void LOG_set_system_file_logging_enabled_state(LOG_system_enum_t system, uint8_t state) {
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system == system) {
            LOG_systems[i].file_logging_enabled = state;
            LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_SINK_ALL, 
                    "%s file logging for %s", 
                    state == 0 ? "Disabled" : "Enabled", 
                    LOG_systems[i].name
            );
            return;
        }
    }

    // System not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "LOG_set_system_file_logging_state(): unknown system: %d", system);
    return;
}

/// @brief Report a sink's enabled state
/// @param sink specified sink
void LOG_report_sink_enabled_state(LOG_sink_enum_t sink) {
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SINKS; i++) {
        if (LOG_sinks[i].sink == sink) {
            LOG_message(
                LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
                "%20s: %s",  LOG_sinks[i].name, LOG_sinks[i].enabled ? "enabled" : "DISABLED"
            );
            return;
        }
    }

    // Sink not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "LOG_report_sink_enabled_state(): unknown sink: %d", sink);

    return;
}

/// @brief Report a subsystem's file-logging state
/// @param sink specified sink
/// @note includes the subsystem's log filename
void LOG_report_system_file_logging_state(LOG_system_enum_t system) {
    for (uint16_t i = 0; i < LOG_NUMBER_OF_SYSTEMS; i++) {
        if (LOG_systems[i].system == system) {
            LOG_message(
                LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
                "%20s: %9s (log file: '%s')",
                LOG_systems[i].name, LOG_systems[i].file_logging_enabled ? "enabled" : "DISABLED",
                LOG_systems[i].log_file_path
            );
            return;
        }
    }

    // System not found
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "LOG_report_system_file_logging_state(): unknown system: %d", system);
    return;
}

uint16_t LOG_number_of_logging_sinks(void)
{
    return LOG_NUMBER_OF_SINKS;
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

/// @brief Get the size in entries of the in-memory log table
/// @return size of the in-memory log table
uint8_t LOG_memory_table_max_entries(void)
{
    return LOG_MEMORY_NUMBER_OF_ENTRIES;
}

/// @brief Get the index of the most recent in-memory log message
/// @return index of the most recent log message
uint8_t LOG_get_memory_table_index_of_most_recent_log_entry(void)
{
    return LOG_memory_index_of_current_log_entry;
}

/// @brief Get a pointer to the full text of the in-memory log message at the
/// specified index
/// @param index requested in-memory log message
/// @return pointer to the full text of the log message
const char *LOG_get_memory_table_full_message_at_index(uint8_t index)
{
    return LOG_memory_table[index].full_message;
}

/// @brief Get the most recent log message text 
/// @return pointer to the full text of the most recent message
/// (statically allocated)
const char *LOG_get_most_recent_log_message_text(void)
{
    return LOG_memory_table[LOG_memory_index_of_current_log_entry].full_message;
}

