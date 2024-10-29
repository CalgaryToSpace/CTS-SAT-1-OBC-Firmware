#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_channel_control.h"
#include "eps_drivers/eps_commands.h"

#include <string.h>

/// @brief Converts an EPS channel name to a channel enum.
/// @param channel_name A lowercase c-string of the channel name (e.g., "mpi"), or a number
/// representing the channel number (e.g., "1" or "16").
/// Valid string values: "vbatt_stack", "stack_5v", "stack_3v3", "camera", "uhf_antenna_deploy",
/// "lora_module", "mpi_5v", "mpi_12v", "boom".
/// @return The corresponding enum value. Returns EPS_CHANNEL_UNKNOWN if no match is found.
EPS_CHANNEL_enum_t EPS_channel_from_str(const char channel_name[])
{
    // Parse Numbers.
    if (strcmp(channel_name, "0") == 0)
        return EPS_CHANNEL_VBATT_STACK;
    if (strcmp(channel_name, "1") == 0)
        return EPS_CHANNEL_5V_STACK;
    if (strcmp(channel_name, "2") == 0)
        return EPS_CHANNEL_5V_CH2_UNUSED;
    if (strcmp(channel_name, "3") == 0)
        return EPS_CHANNEL_5V_CH3_UNUSED;
    if (strcmp(channel_name, "4") == 0)
        return EPS_CHANNEL_5V_MPI;
    if (strcmp(channel_name, "5") == 0)
        return EPS_CHANNEL_3V3_STACK;
    if (strcmp(channel_name, "6") == 0)
        return EPS_CHANNEL_3V3_CAMERA;
    if (strcmp(channel_name, "7") == 0)
        return EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY;
    if (strcmp(channel_name, "8") == 0)
        return EPS_CHANNEL_3V3_LORA_MODULE;
    if (strcmp(channel_name, "9") == 0)
        return EPS_CHANNEL_VBATT_CH9_UNUSED;
    if (strcmp(channel_name, "10") == 0)
        return EPS_CHANNEL_VBATT_CH10_UNUSED;
    if (strcmp(channel_name, "11") == 0)
        return EPS_CHANNEL_VBATT_CH11_UNUSED;
    if (strcmp(channel_name, "12") == 0)
        return EPS_CHANNEL_12V_MPI;
    if (strcmp(channel_name, "13") == 0)
        return EPS_CHANNEL_12V_BOOM;
    if (strcmp(channel_name, "14") == 0)
        return EPS_CHANNEL_3V3_CH14_UNUSED;
    if (strcmp(channel_name, "15") == 0)
        return EPS_CHANNEL_3V3_CH15_UNUSED;
    if (strcmp(channel_name, "16") == 0)
        return EPS_CHANNEL_28V6_CH16_UNUSED;

    // Parse Strings.
    if (strcmp(channel_name, "vbatt_stack") == 0)
        return EPS_CHANNEL_VBATT_STACK;
    if (strcmp(channel_name, "stack_5v") == 0)
        return EPS_CHANNEL_5V_STACK;
    if (strcmp(channel_name, "stack_3v3") == 0)
        return EPS_CHANNEL_3V3_STACK;
    if (strcmp(channel_name, "camera") == 0)
        return EPS_CHANNEL_3V3_CAMERA;
    if (strcmp(channel_name, "uhf_antenna_deploy") == 0)
        return EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY;
    if (strcmp(channel_name, "lora_module") == 0)
        return EPS_CHANNEL_3V3_LORA_MODULE;
    if (strcmp(channel_name, "mpi_5v") == 0)
        return EPS_CHANNEL_5V_MPI;
    if (strcmp(channel_name, "mpi_12v") == 0)
        return EPS_CHANNEL_12V_MPI;
    if (strcmp(channel_name, "boom") == 0)
        return EPS_CHANNEL_12V_BOOM;

    return EPS_CHANNEL_UNKNOWN;
}

/// @brief Sets the enabled state of an EPS channel (on or off).
/// @param channel The channel to enable or disable.
/// @param enabled 0 to disable, >0 to enable.
/// @return 0 on success, >0 on failure. Returns 99 if channel is unknown. Propogates the
///     return value from EPS_CMD_output_bus_channel_on/off().
uint8_t EPS_set_channel_enabled(EPS_CHANNEL_enum_t channel, uint8_t enabled)
{
    if (channel == EPS_CHANNEL_UNKNOWN)
    {
        return 99;
    }

    if (enabled)
    {
        return EPS_CMD_output_bus_channel_on(channel);
    }
    else
    {
        return EPS_CMD_output_bus_channel_off(channel);
    }
    return 100; // Unreachable
}
