
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "log/log.h"
#include "log/log_sinks.h"
#include "telecommands/log_telecommand_defs.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "telecommand_exec/telecommand_types.h"


/// @brief Telecommand: Set a LOG sink's enabled state
/// @param args_str
/// - Arg 0: Sink enum value
/// - Arg 1: Enabled? 0: disable sink, 1: enable sink
/// @details FrontierSat LOG sinks
///    LOG_SINK_UHF_RADIO = 1
///    LOG_SINK_FILE = 2 
///    LOG_SINK_UMBILICAL_UART = 4
uint8_t TCMDEXEC_log_set_sink_enabled_state(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t sink;
    uint8_t sink_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &sink);
    if (sink_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Unable to parse sink from first telecommand argument");
        return 1;
    }

    uint64_t state;
    uint8_t state_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &state);
    if (state_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Unable to parse state from second telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_set_sink_enabled_state(sink, state);

    return 0;
}

/// @brief Telecommand: Set a LOG subsystem's file logging enabled state
/// @param args_str
/// - Arg 0: Subsystem enum value
/// - Arg 1: Enabled? 0: disable file logging, 1: enable file logging
/// @details FrontierSat LOG sinks
///    LOG_SYSTEM_OBC = 1
///    LOG_SYSTEM_UHF_RADIO = 2
///    LOG_SYSTEM_UMBILICAL_UART = 4
///    LOG_SYSTEM_GNSS = 8
///    LOG_SYSTEM_MPI = 16
///    LOG_SYSTEM_EPS = 32
///    LOG_SYSTEM_BOOM = 64
///    LOG_SYSTEM_ADCS = 128
///    LOG_SYSTEM_LFS = 256
///    LOG_SYSTEM_FLASH = 512
///    LOG_SYSTEM_ANTENNA_DEPLOY = 1024
///    LOG_SYSTEM_LOG = 2048
///    LOG_SYSTEM_TELECOMMAND = 4096
///    LOG_SYSTEM_UNIT_TEST = 8192
uint8_t TCMDEXEC_log_set_system_file_logging_enabled_state(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t system;
    uint8_t system_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &system);
    if (system_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Unable to parse system from first telecommand argument");
        return 1;
    }

    uint64_t state;
    uint8_t state_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &state);
    if (state_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Unable to parse state from second telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_set_system_file_logging_enabled_state(system, state);

    return 0;
}

/// @brief Telecommand: Report a LOG sink's enabled state
/// @param args_str
/// - Arg 0: Sink enum
uint8_t TCMDEXEC_log_report_sink_enabled_state(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t sink;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &sink);
    if (result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Unable to parse sink from telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_report_sink_enabled_state(sink);

    return 0;
}

/// @brief Telecommand: Report all LOG sink enable states
uint8_t TCMDEXEC_log_report_all_sink_enabled_states(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    // Response is logged by log system
    for (uint16_t i = 0; i < LOG_number_of_logging_sinks(); i++) {
        LOG_report_sink_enabled_state(1 << i);
    }

    return 0;
}

/// @brief Telecommand: Report LOG subsystem's file logging state (and show
///     logging filename)
/// @param args_str
/// - Arg 0: Subsystem enum
uint8_t TCMDEXEC_log_report_system_file_logging_state(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t system;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &system);
    if (result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Unable to parse system from telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_report_system_file_logging_state(system);

    return 0;
}

/// @brief Telecommand: Report all LOG subsystem file logging states
uint8_t TCMDEXEC_log_report_all_system_file_logging_states(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    // Response is logged by log system
    // Do not include the "unknown", which is the last entry
    for (uint16_t i = 0; i < LOG_number_of_logging_systems() - 1; i++) {
        LOG_report_system_file_logging_state(1 << i);
    }

    return 0;
}

/// @brief Telecommand: Enable or disable debugging messages for LOG sink
/// @param args_str
/// - Arg 0: Sink enum
/// - Arg 1: State 0: disable debug messages, 1: enable debug messages
uint8_t TCMDEXEC_log_set_sink_debugging_messages_state(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t sink = 0;
    uint8_t sink_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &sink);
    if (sink_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Unable to parse sink from first telecommand argument");
        return 1;
    }
    
    uint64_t state = 0;
    uint8_t state_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &state);
    if (state_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Unable to parse state from second telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_set_sink_debugging_messages_enabled_state(sink, state);

    return 0;
}

/// @brief Telecommand: Enable or disable debugging messages for LOG subsystem
/// @param args_str
/// - Arg 0: Subsystem enum
/// - Arg 1: State 0: disable debug messages, 1: enable debug messages
uint8_t TCMDEXEC_log_set_system_debugging_messages_state(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t system = 0;
    uint8_t system_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &system);
    if (system_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Unable to parse system from first telecommand argument");
        return 1;
    }
    
    uint64_t state = 0;
    uint8_t state_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &state);
    if (state_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Unable to parse state from second telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_set_system_debugging_messages_enabled_state(system, state);

    return 0;
}

/// @brief Telecommand: Report the latest log message to the incoming
/// telecommand channel
uint8_t TCMDEXEC_log_report_latest_message_from_memory(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    // This does not get logged. Send result directly back on telecommand
    // channel the request arrived on
    const char *log_text = LOG_get_most_recent_log_message_text();
    switch (tcmd_channel) {
        case TCMD_TelecommandChannel_DEBUG_UART:
            LOG_to_umbilical_uart(log_text);
            break;
        case TCMD_TelecommandChannel_RADIO1:
            LOG_to_uhf_radio(log_text);
            break;
        default:
            // Channel is unknown, log just to the filesystem
            LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_FILE, "TCMDEXEC_log_report_latest_message(): Unknown telecommand channel");
            break;
    }

    return 0;
}

/// @brief Telecommand: Report the N latest log messages to the incoming
/// telecommand channel
/// @param args_str
/// - Arg 0: Number of latest log messages to report
uint8_t TCMDEXEC_log_report_n_latest_messages_from_memory(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    // This does not get logged. Send result directly back on telecommand
    // channel the request arrived on
    uint64_t requested_number_of_entries = 0;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &requested_number_of_entries);
    if (result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "TCMDEXEC_log_report_n_latest_messages(): Unable to parse number of entries from first telecommand argument");
        return 1;
    }
    const uint8_t max_entries = LOG_memory_table_max_entries();
    if (requested_number_of_entries > max_entries) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "TCMDEXEC_log_report_n_latest_messages(): Requested number of log messages must be at most %d", max_entries);
        return 1;
    }

    // Report the entries in the log table, which is a circular buffer
    uint8_t current_index = LOG_get_memory_table_index_of_most_recent_log_entry();
    int16_t signed_start_index = ((int16_t)current_index + 1 - (int16_t)requested_number_of_entries); 
    if (signed_start_index < 0) {
        signed_start_index += max_entries;
    }
    uint16_t start_index = (uint16_t)signed_start_index;

    void (*transmit_function)(const char *);
    switch (tcmd_channel) {
        case TCMD_TelecommandChannel_DEBUG_UART:
            transmit_function = LOG_to_umbilical_uart;
            break;
        case TCMD_TelecommandChannel_RADIO1:
            transmit_function = LOG_to_uhf_radio;
            break;
        default:
            // Channel is unknown, log error to the filesystem and return
            LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_FILE, "TCMDEXEC_log_report_latest_message(): Unknown telecommand channel");
            return 1;
            break;
    }

    // Report the messages
    for (uint16_t response_number = 0; response_number < requested_number_of_entries; response_number++) {
        const uint16_t log_index = (start_index + response_number) % max_entries;
        const char *log_text = LOG_get_memory_table_full_message_at_index(log_index);
        transmit_function(log_text);
    }
    
    return 0;
}
