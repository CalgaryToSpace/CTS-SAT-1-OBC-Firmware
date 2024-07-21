
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "log/log.h"
#include "telecommands/log_telecommand_defs.h"
#include "telecommands/telecommand_args_helpers.h"


/// @brief Telecommand: Set a LOG sink's enabled state
/// @param args_str
/// - Arg 0: Sink enum value
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
/// @details FrontierSat LOG sinks
///    LOG_SYSTEM_OBC = 1
///    LOG_SYSTEM_UHF_RADIO = 2
///    LOG_SYSTEM_UMBILICAL_UART = 4
///    LOG_SYSTEM_GPS = 8
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

