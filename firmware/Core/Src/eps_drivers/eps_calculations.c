#include "eps_drivers/eps_calculations.h"
#include "eps_drivers/eps_channel_control.h"
#include "eps_drivers/eps_types.h"
#include <stddef.h>
#include <stdio.h>

/// @brief Calculate the current EPS battery voltage from a PBU telemetry struct.
/// @param battery Telemetry struct
/// @return Battery percentage, as a float. Nominally, the battery percentage is between 0 and 100.
///     However, the battery percentage can exceed 100% if the battery voltage is above the maximum
///     voltage, and can be less than 0% if the battery voltage is below the minimum voltage.
float EPS_convert_battery_voltage_to_percent(EPS_battery_pack_datatype_eng_t battery) {

    const uint16_t min_total_voltage_mV = 12000;
    const uint16_t max_total_voltage_mV = 16000;

    const int16_t battery_total_voltage_mV = battery.vip_bp_input.voltage_mV; 
    
    const float calc = (
        ((float)(battery_total_voltage_mV - min_total_voltage_mV))
        / ((float)(max_total_voltage_mV - min_total_voltage_mV))
    );

    // convert to percent
    return calc * 100.0;
}

/// @brief Sum the total fault count for all channels.
/// @param fault_state A pointer to the fault state struct.
/// @return The summed fault count for all channels.
int32_t EPS_calculate_total_fault_count(EPS_struct_pdu_overcurrent_fault_state_t *fault_state) {
    int32_t fault_count = 0;

    for (uint8_t ch_num = 0; ch_num < EPS_TOTAL_CHANNEL_COUNT; ch_num++) {
        fault_count += fault_state->overcurrent_fault_count_each_channel[ch_num];
    }

    return fault_count;
}

/// @brief Calculate the difference in overcurrent fault counts for all channels and determine powered channels.
/// @param before Pointer to the fault state before the event.
/// @param after Pointer to the fault state after the event.
/// @param comparison Pointer to the struct where the difference result will be stored.
/// @return 0 on success, 1 on invalid input, 2 if no difference.
uint8_t EPS_calculate_overcurrent_difference(
    const EPS_struct_pdu_overcurrent_fault_state_t *before,
    const EPS_struct_pdu_overcurrent_fault_state_t *after,
    EPS_struct_pdu_overcurrent_fault_comparison_t *comparison)
{
    if (!before || !after || !comparison) {
        return 1;  // Invalid input
    }

    // Initialize comparison struct
    comparison->total_difference = 0;
    comparison->channels_with_new_faults = 0;
    comparison->channels_with_new_faults_bitfield = 0;

    // Initialize powered channel JSON strings
    comparison->powered_channels_before_json[0] = '\0';
    comparison->powered_channels_after_json[0] = '\0';

    // Get powered channels before and after
    char powered_before_json[256] = {0};
    char powered_after_json[256] = {0};

    EPS_get_enabled_channels_json(before->stat_ch_on_bitfield, before->stat_ch_ext_on_bitfield, powered_before_json, sizeof(powered_before_json));
    EPS_get_enabled_channels_json(after->stat_ch_on_bitfield, after->stat_ch_ext_on_bitfield, powered_after_json, sizeof(powered_after_json));

    // Store the powered channels in the comparison struct
    snprintf(comparison->powered_channels_before_json, sizeof(comparison->powered_channels_before_json), "%s", powered_before_json);
    snprintf(comparison->powered_channels_after_json, sizeof(comparison->powered_channels_after_json), "%s", powered_after_json);

    // Initialize total fault counts
    uint16_t total_fault_count_before = 0;
    uint16_t total_fault_count_after = 0;

    // Calculate differences for each channel
    for (int i = 0; i < 32; i++) {
        uint16_t before_count = before->overcurrent_fault_count_each_channel[i];
        uint16_t after_count = after->overcurrent_fault_count_each_channel[i];

        uint16_t difference = (after_count >= before_count) ? (after_count - before_count) : 0;

        comparison->difference_each_channel[i] = difference;
        comparison->total_difference += difference;

        if (difference > 0) {
            comparison->channels_with_new_faults++;
            comparison->channels_with_new_faults_bitfield |= (1 << i);
        }

        // Update the total fault counts
        total_fault_count_before += before_count;
        total_fault_count_after += after_count;
    }

    // Store the total fault counts in the comparison struct
    comparison->total_fault_count_before = total_fault_count_before;
    comparison->total_fault_count_after = total_fault_count_after;

    // Determine power channel status
    if (comparison->channels_with_new_faults > 0) {
        snprintf(comparison->power_channel_status, sizeof(comparison->power_channel_status), "FAILED");
    } else {
        snprintf(comparison->power_channel_status, sizeof(comparison->power_channel_status), "OK");
    }

    // Return 0 for success
    return 0;
}
