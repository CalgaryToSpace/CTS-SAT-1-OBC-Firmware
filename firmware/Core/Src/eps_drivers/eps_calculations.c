#include "eps_drivers/eps_calculations.h"
#include "eps_drivers/eps_types.h"
#include <stddef.h>

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

/// @brief Calculate the difference in overcurrent fault counts for all channels.
/// @param before Pointer to the fault state before the event.
/// @param after Pointer to the fault state after the event.
/// @param result Pointer to the struct where the difference result will be stored.
/// @return 0 on success, 1 on invalid input, 2 if no difference.
uint8_t EPS_calculate_overcurrent_difference(
    const EPS_struct_pdu_overcurrent_fault_state_t *before,
    const EPS_struct_pdu_overcurrent_fault_state_t *after,
    EPS_struct_pdu_overcurrent_fault_comparison_t *result)
{
    if (before == NULL || after == NULL || result == NULL) {
        return 1;  // Invalid input
    }

    result->before_power_on = *before;
    result->after_power_on  = *after;

    result->total_difference = 0;
    result->channels_with_new_faults = 0;

    for (int i = 0; i < 32; i++) {
        uint16_t before_count = before->overcurrent_fault_count_each_channel[i];
        uint16_t after_count  = after->overcurrent_fault_count_each_channel[i];

        // Avoid underflow: if after < before, set difference to 0
        uint16_t difference;
        if (after_count >= before_count) {
            difference = after_count - before_count;
        } else {
            difference = 0;
        }

        result->difference_each_channel[i] = difference;
        result->total_difference += difference;

        if (difference > 0) {
            result->channels_with_new_faults++;
        }
    }

    if (result->total_difference == 0) {
        return 2;  // No difference
    }

    return 0;  // Success
}
