
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "log/log.h"
#include "telecommands/log_telecommand_defs.h"
#include "telecommands/telecommand_args_helpers.h"


uint8_t TCMDEXEC_log_set_channel_state(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t channel;
    uint8_t channel_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &channel);
    if (channel_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse channel from first telecommand argument");
        return 1;
    }

    uint64_t state;
    uint8_t state_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &state);
    if (state_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse state from second telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_set_channel_state(channel, state);

    return 0;
}

uint8_t TCMDEXEC_log_set_file_logging_state_for_system(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t system;
    uint8_t system_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &system);
    if (system_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse system from first telecommand argument");
        return 1;
    }

    uint64_t state;
    uint8_t state_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &state);
    if (state_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse state from second telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_set_file_logging_state_for_system(system, state);

    return 0;
}

uint8_t TCMDEXEC_log_report_channel_logging_state(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t channel;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &channel);
    if (result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse channel from telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_report_channel_logging_state(channel);

    return 0;
}

uint8_t TCMDEXEC_log_report_system_file_logging_state(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t system;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &system);
    if (result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse system from telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_report_system_file_logging_state(system);

    return 0;
}

uint8_t TCMDEXEC_log_set_channel_debugging_messages_state(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t channel = 0;
    uint8_t channel_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &channel);
    if (channel_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse channel from first telecommand argument");
        return 1;
    }
    
    uint64_t state = 0;
    uint8_t state_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &state);
    if (state_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse state from second telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_set_channel_debugging_messages_state(channel, state);

    return 0;
}

uint8_t TCMDEXEC_log_set_system_debugging_messages_state(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t system = 0;
    uint8_t system_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &system);
    if (system_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse system from first telecommand argument");
        return 1;
    }
    
    uint64_t state = 0;
    uint8_t state_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &state);
    if (state_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse state from second telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_set_system_debugging_messages_state(system, state);

    return 0;
}

