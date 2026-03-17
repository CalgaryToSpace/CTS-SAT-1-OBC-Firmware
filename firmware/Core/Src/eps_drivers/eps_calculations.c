#include "eps_drivers/eps_calculations.h"

/// @brief Calculate the current EPS battery voltage from a PBU telemetry struct.
/// @param battery Telemetry struct
/// @return Battery percentage, as a float. Nominally, the battery percentage is between 0 and 100.
///     However, the battery percentage can exceed 100% if the battery voltage is above the maximum
///     voltage, and can be less than 0% if the battery voltage is below the minimum voltage.
float EPS_convert_battery_voltage_to_percent(EPS_battery_pack_datatype_eng_t battery) {
    // Source (low side) - 12.4V:
    // SAFETY_VOLT_LOTHR on Page 93 of EPS Software ICD.
    // EMLOPO_VOLT_HITHR on Page 99 of EPS Software ICD.
    const uint16_t min_total_voltage_mV = 12400;
    
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


/// @brief Sum the total solar panel input for all channels.
/// @return Total input of all PCU channels in cW.
int32_t EPS_calculate_total_pcu_power_input_cW(EPS_struct_pcu_housekeeping_data_eng_t *pcu_data) {
    int32_t total_cW = 0;

    for (uint8_t ch_num = 0; ch_num < EPS_TOTAL_PCU_CHANNEL_COUNT; ch_num++) {
        const EPS_conditioning_channel_datatype_eng_t *pcu_channel_data = (
            &pcu_data->conditioning_channel_info_each_channel[ch_num]
        );

        // Convert to cW.
        // Example: 5000 mV x 1000 mA = 5,000,000 ==> 5,000,000 x 10^-4 = 500 cW = 5 W
        total_cW += (
            (float)pcu_channel_data->volt_in_mppt_mV * (float)pcu_channel_data->curr_in_mppt_mA * 1e-4
        );
    }

    return total_cW;
}


/// @brief Sum the total solar panel output for all channels.
/// @return Total output of all PCU channels in cW.
int32_t EPS_calculate_total_pcu_power_output_cW(EPS_struct_pcu_housekeeping_data_eng_t *pcu_data) {
    int32_t total_cW = 0;

    for (uint8_t ch_num = 0; ch_num < EPS_TOTAL_PCU_CHANNEL_COUNT; ch_num++) {
        const EPS_conditioning_channel_datatype_eng_t *pcu_channel_data = (
            &pcu_data->conditioning_channel_info_each_channel[ch_num]
        );

        // Convert to cW.
        // Example: 5000 mV x 1000 mA = 5,000,000 ==> 5,000,000 x 10^-4 = 500 cW = 5 W
        total_cW += (
            (float)pcu_channel_data->volt_ou_mppt_mV * (float)pcu_channel_data->curr_ou_mppt_mA * 1e-4
        );
    }

    return total_cW;
}
