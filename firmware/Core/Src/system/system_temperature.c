#include "main.h"

#include "stdio.h"
#include <stdint.h>

#include "system/system_temperature.h"
#include "obc_temperature_sensor/obc_temperature_sensor.h"
#include "antenna_deploy_drivers/ant_commands.h"
#include "eps_drivers/eps_calculations.h"
#include "eps_drivers/eps_commands.h"
#include "obc_temperature_sensor/obc_temperature_sensor.h"

/// @brief Grabs required data/data structures from drivers containing thermal information.
/// @param result Struct that contains various raw thermal data.
/// @return 0 if data was successfully stored, 1 if error unable to get data.
uint8_t SYS_TEMP_get_raw_thermal_info(SYS_TEMP_raw_thermal_info_t* result){

    int32_t obc_temp_result = OBC_TEMP_SENSOR_get_temperature_cC();

    uint8_t stat;
    
    uint16_t ANT_raw_temp_A;
    stat = ANT_CMD_measure_temp(ANT_I2C_BUS_A_MCU_A, &ANT_raw_temp_A);
    if (stat!=0){
        return 1;
    }

    uint16_t ANT_raw_temp_B;
    stat = ANT_CMD_measure_temp(ANT_I2C_BUS_B_MCU_B, &ANT_raw_temp_B);
    if (stat!=0){
        return 1;
    }

    //get solar panel (PCU, mppt)
    EPS_struct_pcu_housekeeping_data_eng_t pcu_data;
    stat = EPS_CMD_get_pcu_housekeeping_data_run_avg(&pcu_data);
    if (stat!=0){
        return 1;
    }
    
    // get battery unit data
    EPS_struct_pbu_housekeeping_data_eng_t pbu_data;
    stat = EPS_CMD_get_pbu_housekeeping_data_eng(&pbu_data);
    if (stat!=0) {
        return 1;
    }

    result->system_OBC_temperature_cC = obc_temp_result;

    result->system_ANT_temperature_i2c_bus_A_raw = ANT_raw_temp_A;
    result->system_ANT_temperature_i2c_bus_B_raw = ANT_raw_temp_B;

    result->system_eps_battery_datatype_struct = pbu_data.battery_pack_info_each_pack[0];
    
    for (int i =0; i< EPS_COND_CHANNEL_SIZE; i++){
        result->system_eps_conditioning_channel_info_each_channel[i] = pcu_data.conditioning_channel_info_each_channel[i];
    }

    return 0;
}

/// @brief Takes the raw thermal data and converts them to human readable values.
/// @param input The structure containing various raw thermal data. 
/// @param result The structure containing the processed thermal data. 
/// @return 0 on successful conversion, otherwise failed.
uint8_t SYS_TEMP_pack_to_system_thermal_info(SYS_TEMP_raw_thermal_info_t* input, SYS_TEMP_thermal_info_t* result) {

    result->system_OBC_temperature_cC = input->system_OBC_temperature_cC ;

    result->system_ANT_temperature_i2c_bus_A_cC = ANT_convert_raw_temp_to_cCelsius(input->system_ANT_temperature_i2c_bus_A_raw);
    result->system_ANT_temperature_i2c_bus_B_cC = ANT_convert_raw_temp_to_cCelsius(input->system_ANT_temperature_i2c_bus_B_raw);

    for (int i = 0; i < EPS_COND_CHANNEL_SIZE; i++) { 
        result->system_solar_panel_power_generation_mW[i] = (int32_t) (input->system_eps_conditioning_channel_info_each_channel[i].volt_in_mppt_mV * input->system_eps_conditioning_channel_info_each_channel[i].curr_in_mppt_mA );
    } 

    result->system_eps_battery_percent = EPS_convert_battery_voltage_to_percent(input->system_eps_battery_datatype_struct);
    uint16_t bp_status_bitfield = input->system_eps_battery_datatype_struct.bp_status_bitfield;
    result->system_eps_battery_heater_status_bit = (bp_status_bitfield >> 12) & 1;
    for (int i = 0; i < EPS_BATTERY_PACK_SENSOR_SIZE; i++) { 
        result->system_eps_battery_each_sensor_temperature_cC[i] = input->system_eps_battery_datatype_struct.battery_temperature_each_sensor_cC[i];
    }

    return 0;
}

