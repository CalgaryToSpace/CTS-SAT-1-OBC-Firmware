#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_channel_control.h"
#include "eps_drivers/eps_commands.h"

#include <string.h>
#include <stdio.h>

/// @brief Converts an EPS channel name to a channel enum.
/// @param channel_name A lowercase c-string of the channel name (e.g., "mpi"), or a number
/// representing the channel number (e.g., "1" or "16").
/// Valid string values: "vbatt_stack", "stack_5v", "stack_3v3", "camera", "uhf_antenna_deploy",
/// "gnss", "mpi_5v", "mpi_12v", "boom".
/// @return The corresponding enum value. Returns EPS_CHANNEL_UNKNOWN if no match is found.
EPS_CHANNEL_enum_t EPS_channel_from_str(const char channel_name[]) {
    // Parse Numbers.
    if (strcmp(channel_name, "0") == 0) return EPS_CHANNEL_VBATT_STACK;
    if (strcmp(channel_name, "1") == 0) return EPS_CHANNEL_5V_STACK;
    if (strcmp(channel_name, "2") == 0) return EPS_CHANNEL_5V_CH2_UNUSED;
    if (strcmp(channel_name, "3") == 0) return EPS_CHANNEL_5V_CH3_UNUSED;
    if (strcmp(channel_name, "4") == 0) return EPS_CHANNEL_5V_MPI;
    if (strcmp(channel_name, "5") == 0) return EPS_CHANNEL_3V3_STACK;
    if (strcmp(channel_name, "6") == 0) return EPS_CHANNEL_3V3_CAMERA;
    if (strcmp(channel_name, "7") == 0) return EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY;
    if (strcmp(channel_name, "8") == 0) return EPS_CHANNEL_3V3_LORA_MODULE; // Engg Model ADCS
    if (strcmp(channel_name, "9") == 0) return EPS_CHANNEL_VBATT_CH9_UNUSED;
    if (strcmp(channel_name, "10") == 0) return EPS_CHANNEL_VBATT_CH10_UNUSED;
    if (strcmp(channel_name, "11") == 0) return EPS_CHANNEL_VBATT_CH11_UNUSED;
    if (strcmp(channel_name, "12") == 0) return EPS_CHANNEL_12V_MPI;
    if (strcmp(channel_name, "13") == 0) return EPS_CHANNEL_12V_BOOM;
    if (strcmp(channel_name, "14") == 0) return EPS_CHANNEL_3V3_CH14_UNUSED;
    if (strcmp(channel_name, "15") == 0) return EPS_CHANNEL_3V3_CH15_UNUSED;
    if (strcmp(channel_name, "16") == 0) return EPS_CHANNEL_28V6_CH16_UNUSED;
    
    // Parse Strings.
    if (strcmp(channel_name, "vbatt_stack") == 0) return EPS_CHANNEL_VBATT_STACK;
    if (strcmp(channel_name, "stack_5v") == 0) return EPS_CHANNEL_5V_STACK;
    if (strcmp(channel_name, "stack_3v3") == 0) return EPS_CHANNEL_3V3_STACK;
    if (strcmp(channel_name, "camera") == 0) return EPS_CHANNEL_3V3_CAMERA;
    if (strcmp(channel_name, "uhf_antenna_deploy") == 0) return EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY;
    if (strcmp(channel_name, "gnss") == 0) return EPS_CHANNEL_3V3_LORA_MODULE;
    if (strcmp(channel_name, "mpi_5v") == 0) return EPS_CHANNEL_5V_MPI;
    if (strcmp(channel_name, "mpi_12v") == 0) return EPS_CHANNEL_12V_MPI;
    if (strcmp(channel_name, "boom") == 0) return EPS_CHANNEL_12V_BOOM;
    
    return EPS_CHANNEL_UNKNOWN;
}

/// @brief Converts channel enum into an ASCII-printable name string.
/// @param channel An enum of typedef EPS_CHANNEL_enum_t representing the different channel names of the EPS
/// Ex. EPS_CHANNEL_VBATT_STACK , EPS_CHANNEL_5V_CH2_UNUSED or EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY
/// @return Returns a string of the channel name corresponding to the inputted channel enum
char* EPS_channel_to_str(EPS_CHANNEL_enum_t channel) {
    switch (channel)
    {
    case EPS_CHANNEL_VBATT_STACK:
        return "VBATT_STACK";
    case EPS_CHANNEL_5V_STACK:
        return "5V_STACK";
    case EPS_CHANNEL_5V_CH2_UNUSED:
        return "5V_CH2_UNUSED";
    case EPS_CHANNEL_5V_CH3_UNUSED:
        return "5V_CH3_UNUSED";
    case EPS_CHANNEL_5V_MPI:
        return "5V_MPI";
    case EPS_CHANNEL_3V3_STACK:
        return "3V3_STACK";
    case EPS_CHANNEL_3V3_CAMERA:
        return "3V3_CAMERA";
    case EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY:
        return "3V3_UHF_ANTENNA_DEPLOY";
    case EPS_CHANNEL_3V3_LORA_MODULE:
        return "3V3_LORA_MODULE";
    case EPS_CHANNEL_VBATT_CH9_UNUSED:
        return "VBATT_CH9_UNUSED";
    case EPS_CHANNEL_VBATT_CH10_UNUSED:
        return "VBATT_CH10_UNUSED";
    case EPS_CHANNEL_VBATT_CH11_UNUSED:
        return "VBATT_CH11_UNUSED";
    case EPS_CHANNEL_12V_MPI:
        return "12V_MPI";
    case EPS_CHANNEL_12V_BOOM:
        return "12V_BOOM";
    case EPS_CHANNEL_3V3_CH14_UNUSED:
        return "3V3_CH14_UNUSED";
    case EPS_CHANNEL_3V3_CH15_UNUSED:
        return "3V3_CH15_UNUSED";
    case EPS_CHANNEL_28V6_CH16_UNUSED:
        return "28V6_CH16_UNUSED";
    case EPS_CHANNEL_UNKNOWN:
        return "CHANNEL_UNKOWN";
    }
    return "INVALID_CHANNEL";
}

/// @brief Sets the enabled state of an EPS channel (on or off).
/// @param channel The channel to enable or disable.
/// @param enabled 0 to disable, >0 to enable.
/// @return 0 on success, >0 on failure. Returns 99 if channel is unknown. Propagates the
///     return value from EPS_CMD_output_bus_channel_on/off().
uint8_t EPS_set_channel_enabled(EPS_CHANNEL_enum_t channel, uint8_t enabled) {
    if (channel == EPS_CHANNEL_UNKNOWN) {
        return 99;
    }
    if (enabled) {
        return EPS_CMD_output_bus_channel_on(channel);
    }
    else {
        return EPS_CMD_output_bus_channel_off(channel);
    }
    return 100; // Unreachable
}

/// @brief Takes in the status bitfields of the eps and outputs the status of the given channel number
/// @param status_bitfield_1 The status bitfield of channels 0-15.
/// @param status_bitfield_2 The status bitfield of channels 16-31.
/// @return 0 if the channel is disabled, 1 if the channel is enabled, and 2 if channel number is invalid
/// @note If all channels are enabled, the output buffer would be require about 300 bytes.
uint8_t EPS_check_status_bit_of_channel(
    uint16_t status_bitfield_1 , uint16_t status_bitfield_2, uint8_t channel_number
) {
    if (channel_number < 16) {
        return ((status_bitfield_1 >> channel_number) & 1);
    } else if  (channel_number < 32) {
        return ((status_bitfield_2 >> (channel_number%16)) & 1);
    } else {
        return 2; // channel_number inputted to is out of bounds of status bitfields
    }
}

/// @brief Convert EPS ch_number to ch_name, then append as a JSON list element to response_output_buf
/// @param response_output_buf The output buffer that will store the string
/// @param response_output_buf_len The total length of the output buffer
/// @param ch_number The channel number of the EPS that will be converted to channel name
/// @return 0 when string was properly outputted into the buffer, 1 if the channel number is invalid
uint8_t EPS_convert_ch_num_to_string_and_append(
    char *response_output_buf,
    uint16_t response_output_buf_len,
    uint8_t ch_number
) {
    if (ch_number > 16) {
        return 1;
    }
    strncat(response_output_buf, "\"", response_output_buf_len - strlen(response_output_buf) - 1);
    strncat(response_output_buf, EPS_channel_to_str(ch_number), response_output_buf_len - strlen(response_output_buf) - 1);
    strncat(response_output_buf, "\"", response_output_buf_len - strlen(response_output_buf) - 1);
    return 0;
}

void EPS_get_enabled_channels_json(
    uint16_t status_bitfield_1, uint16_t status_bitfield_2,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    snprintf(response_output_buf, response_output_buf_len, "[");
    for (uint8_t i = 0; i <= EPS_MAX_ACTIVE_CHANNEL_NUMBER; i++) {
        if (EPS_check_status_bit_of_channel(status_bitfield_1, status_bitfield_2, i)) {
            if (i != 0) {
                strncat(response_output_buf, ",", response_output_buf_len - strlen(response_output_buf) - 1);
            }
            EPS_convert_ch_num_to_string_and_append(response_output_buf, response_output_buf_len, i);
        }
    }
    strncat(response_output_buf, "]", response_output_buf_len - strlen(response_output_buf) - 1);
}
