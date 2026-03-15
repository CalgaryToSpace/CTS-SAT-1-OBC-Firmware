#ifndef INCLUDE_GUARD__SYSTEM_TEMPERATURE_H__
#define INCLUDE_GUARD__SYSTEM_TEMPERATURE_H__

#include "eps_drivers/eps_types.h"

#define EPS_BATTERY_PACK_SENSOR_SIZE 3

typedef struct { 
    int32_t system_OBC_temperature_cC;
    int32_t system_ANT_temperature_i2c_bus_A_cC;
    int32_t system_ANT_temperature_i2c_bus_B_cC;
    int32_t solar_panel_power_input_cW[EPS_TOTAL_PCU_CHANNEL_COUNT_MACRO];
    int32_t solar_panel_power_output_total_cW;
    float system_eps_battery_percent;
    uint8_t system_eps_battery_heater_status_bit;
    int16_t system_eps_battery_each_sensor_temperature_cC[EPS_BATTERY_PACK_SENSOR_SIZE];
} SYS_TEMP_thermal_info_t;

typedef struct {
    int32_t system_OBC_temperature_cC;
    uint16_t system_ANT_temperature_i2c_bus_A_raw;
    uint16_t system_ANT_temperature_i2c_bus_B_raw;
    EPS_struct_pcu_housekeeping_data_eng_t eps_pcu_data;
    EPS_battery_pack_datatype_eng_t system_eps_battery_datatype_struct;
} SYS_TEMP_raw_thermal_info_t;

typedef enum {
    SYS_TEMP_ANT_A_STATUS = 1 << 0,
    SYS_TEMP_ANT_B_STATUS = 1 << 1,
    SYS_TEMP_PCU_STATUS = 1 << 2,
    SYS_TEMP_PBU_STATUS = 1 << 3,
} SYS_TEMP_thermal_info_error_enum_t;

uint8_t SYS_TEMP_get_raw_thermal_info(SYS_TEMP_raw_thermal_info_t* result, uint8_t* error_ret);
uint8_t SYS_TEMP_pack_to_system_thermal_info(SYS_TEMP_raw_thermal_info_t* input, SYS_TEMP_thermal_info_t* result, uint8_t error_ret);

#endif
