
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "log/log.h"
#include "telecommands/log_telecommand_defs.h"
#include "telecommands/telecommand_args_helpers.h"


uint8_t TCMDEXEC_log_enable_channels(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t channels;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &channels);
    if (result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse channels from telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_enable_channels(channels);

    return 0;
}

uint8_t TCMDEXEC_log_disable_channels(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t channels;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &channels);
    if (result) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse channels from telecommand argument");
        return 1;
    }
    
    // Response is logged by log system
    LOG_disable_channels(channels);

    return 0;
}

uint8_t TCMDEXEC_log_enable_systems(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t systems;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &systems);
    if (result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse systems from telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_enable_systems(systems);

    return 0;
}

uint8_t TCMDEXEC_log_disable_systems(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t systems;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &systems);
    if (result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse systems from telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_disable_systems(systems);

    return 0;
}

uint8_t TCMDEXEC_log_channels_status(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t channels;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &channels);
    if (result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse channels from telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_channels_status(channels);

    return 0;
}

uint8_t TCMDEXEC_log_systems_status(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t systems;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &systems);
    if (result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse systems from telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_systems_status(systems);

    return 0;
}

uint8_t TCMDEXEC_log_set_channel_debugging_messages_state(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t channels = 0;
    uint8_t channels_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &channels);
    if (channels_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse channels from first telecommand argument");
        return 1;
    }
    
    uint64_t state = 0;
    uint8_t state_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &state);
    if (state_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse state from 2nd telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_set_channel_debugging_messages_state(channels, state);

    return 0;
}

uint8_t TCMDEXEC_log_set_system_debugging_messages_state(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t systems = 0;
    uint8_t systems_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &systems);
    if (systems_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse systems from first telecommand argument");
        return 1;
    }
    
    uint64_t state = 0;
    uint8_t state_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &state);
    if (state_result) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse state from 2nd telecommand argument");
        return 1;
    }

    // Response is logged by log system
    LOG_set_system_debugging_messages_state(systems, state);

    return 0;
}

