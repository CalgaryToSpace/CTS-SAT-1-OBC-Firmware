#include "eps_drivers/eps_calculations.h"


float EPS_convert_battery_voltage_to_percent(EPS_battery_pack_datatype_eng_t battery) {

    const uint8_t min_total_voltage_V = 12;
    const uint8_t max_total_voltage_V = 16;

    const float cell_total_voltage_V = (battery.vip_bp_input.voltage_mV) * 0.001; 
    
    const float calc = (float)(cell_total_voltage_V - min_total_voltage_V) / (max_total_voltage_V - min_total_voltage_V);

    // convert to percent
    return calc * 100;
}