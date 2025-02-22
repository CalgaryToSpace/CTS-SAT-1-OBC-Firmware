#include "eps_drivers/eps_calculations.h"


float EPS_convert_battery_voltage_to_percent(EPS_battery_pack_datatype_eng_t battery) {
    const int16_t cell_1_mV = battery.cell_voltage_each_cell_mV[0];
    const int16_t cell_2_mV = battery.cell_voltage_each_cell_mV[1];
    const int16_t cell_3_mV = battery.cell_voltage_each_cell_mV[2];
    const int16_t cell_4_mV = battery.cell_voltage_each_cell_mV[3];

    const uint8_t min_total_voltage_V = 12;
    const uint8_t max_total_voltage_V = 16;

    const float cell_total_voltage_V = (cell_1_mV + cell_2_mV + cell_3_mV + cell_4_mV) * 0.001; 
    
    const float calc = (float)(cell_total_voltage_V - min_total_voltage_V) / (max_total_voltage_V - min_total_voltage_V);

    // convert to percent
    return calc * 100;
}