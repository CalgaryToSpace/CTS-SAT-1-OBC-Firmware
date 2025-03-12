#include "eps_drivers/eps_calculations.h"


float EPS_convert_battery_voltage_to_percent(EPS_battery_pack_datatype_eng_t battery) {

    const uint16_t min_total_voltage_mV = 12000;
    const uint16_t max_total_voltage_mV = 16000;

    const int16_t battery_total_voltage_mV = battery.vip_bp_input.voltage_mV; 
    
    const float calc = ((float)(battery_total_voltage_mV - min_total_voltage_mV)) / ((float)(max_total_voltage_mV - min_total_voltage_mV));

    // convert to percent
    return calc * 100.0;
}