
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "log/log.h"
#include "telecommands/log_telecommand_defs.h"
#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"


uint8_t TCMDEXEC_log_enable_channels(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t channels;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &channels);
    if (result) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse channels from telecommand argument");
        return 1;
    }
    // Response is provided by logging system
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
    // Response is provided by logging system
    LOG_disable_channels(channels);

    return 0;
}

uint8_t TCMDEXEC_log_enable_systems(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t systems;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &systems);
    if (result) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse systems from telecommand argument");
        return 1;
    }
    // Response is provided by logging system
    LOG_enable_systems(systems);

    return 0;
}

uint8_t TCMDEXEC_log_disable_systems(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t systems;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &systems);
    if (result) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse systems from telecommand argument");
        return 1;
    }
    // Response is provided by logging system
    LOG_disable_systems(systems);

    return 0;
}

uint8_t TCMDEXEC_log_channels_status(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t channels;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &channels);
    if (result) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse channels from telecommand argument");
        return 1;
    }
    // Response is provided by logging system
    LOG_channels_status(channels);

    return 0;
}

uint8_t TCMDEXEC_log_systems_status(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t systems;
    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &systems);
    if (result) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse systems from telecommand argument");
        return 1;
    }
    // Response is provided by logging system
    LOG_systems_status(systems);

    return 0;
}

