#include "system/system_bootup.h"
#include "eps_drivers/eps_channel_control.h"
#include "boom_deploy_drivers/boom_deploy_drivers.h"
#include "log/log.h"

/// @brief Disables systems on bootup (Non-Default EPS Channels, Boom GPIO Pins)
/// @details This function is called during the bootup process
void SYS_disable_systems_bootup()
{
    // Disabling non-default channels

    uint8_t fail_count = 0;
    for (EPS_CHANNEL_enum_t channel_enum_value = EPS_CHANNEL_VBATT_STACK; channel_enum_value < EPS_ACTIVE_CHANNEL_COUNT; channel_enum_value++) {
        if ((channel_enum_value == EPS_CHANNEL_VBATT_STACK) ||
            (channel_enum_value == EPS_CHANNEL_5V_STACK)    ||
            (channel_enum_value == EPS_CHANNEL_3V3_STACK)) {
                continue;
            }
        const uint8_t disable_result = EPS_set_channel_enabled(channel_enum_value, 0);   
        if (disable_result != 0) {
            fail_count++;
            LOG_message(LOG_SYSTEM_EPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Error Disabling Channel: %s. Error: %u", EPS_channel_to_str(channel_enum_value), disable_result);            
        }
    }

    if (fail_count) {
        // not disabling 3 channels, so not counting them
        const uint8_t success_count = EPS_ACTIVE_CHANNEL_COUNT - 3 - fail_count;
        LOG_message(LOG_SYSTEM_EPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Successfully disabled: %u channels. Failed to disable %u channels!", success_count, fail_count);
        return;
    }

    LOG_message(LOG_SYSTEM_EPS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL, "All non-default channels disabled successfully!");

    // Disable GPIO Pins for BOOM

    BOOM_disable_all_burns();
    LOG_message(LOG_SYSTEM_BOOM, LOG_SEVERITY_NORMAL, LOG_SINK_ALL, "All burns disabled!"); 
}