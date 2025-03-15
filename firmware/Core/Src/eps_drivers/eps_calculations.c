#include "eps_drivers/eps_calculations.h"

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