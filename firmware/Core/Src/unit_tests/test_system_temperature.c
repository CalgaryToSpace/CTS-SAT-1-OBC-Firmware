#include "unit_tests/test_system_temperature.h"
#include "unit_tests/unit_test_helpers.h"

#include "system/system_temperature.h"
#include "unit_tests/test_system_temperature.h"

uint8_t TEST_EXEC__SYS_TEMP_get_processed_thermal_info(){

    SYS_TEMP_thermal_info_t result;
    
    // Case 1: Success on regular inputs + Heater bit is enabled
    SYS_TEMP_raw_thermal_info_t case1;
    uint8_t err_case_1 = 0;
    
    case1.system_OBC_temperature_cC=5000;
    case1.system_ANT_temperature_i2c_bus_A_raw=500;
    case1.system_ANT_temperature_i2c_bus_B_raw=500;

    EPS_vpid_eng_t vpid_case1 = (EPS_vpid_eng_t) {15000,0,0};
    // 0b1001_0001_1110_0001 = 37345 the bit string for batter pack bitfield
    case1.system_eps_battery_datatype_struct = (EPS_battery_pack_datatype_eng_t) {vpid_case1, 37345 ,{0,0,0,0},{400,0,550}};
    case1.system_eps_conditioning_channel_info_each_channel[0] = (EPS_conditioning_channel_datatype_eng_t) {vpid_case1, 1000,1000,300,1000};
    case1.system_eps_conditioning_channel_info_each_channel[1] = (EPS_conditioning_channel_datatype_eng_t) {vpid_case1, 200,500,1000,1000};
    case1.system_eps_conditioning_channel_info_each_channel[2] = (EPS_conditioning_channel_datatype_eng_t) {vpid_case1, 100,100,1000,1000};
    case1.system_eps_conditioning_channel_info_each_channel[3] = (EPS_conditioning_channel_datatype_eng_t) {vpid_case1, 1,5,1000,1000};

    SYS_TEMP_pack_to_system_thermal_info(&case1 , &result, err_case_1);
    TEST_ASSERT(result.system_OBC_temperature_cC == 5000);
    TEST_ASSERT(result.system_ANT_temperature_i2c_bus_A_cC == 4513);
    TEST_ASSERT(result.system_ANT_temperature_i2c_bus_B_cC == 4513);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[0] == 1000000);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[1] == 100000);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[2] == 10000);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[3] == 5);
    TEST_ASSERT(result.system_eps_battery_heater_status_bit == 1);
    TEST_ASSERT(result.system_eps_battery_percent == 75);
    TEST_ASSERT(result.system_eps_battery_each_sensor_temperature_cC[0]==400);
    TEST_ASSERT(result.system_eps_battery_each_sensor_temperature_cC[1]==0);
    TEST_ASSERT(result.system_eps_battery_each_sensor_temperature_cC[2]==550);



    //Case 2: Success on regular inputs + Heater bit is disabled
    SYS_TEMP_raw_thermal_info_t case2;
    uint8_t err_case_2 = 0;
    
    case2.system_OBC_temperature_cC=1000;
    case2.system_ANT_temperature_i2c_bus_A_raw=450;
    case2.system_ANT_temperature_i2c_bus_B_raw=500;
    
    EPS_vpid_eng_t vpid_case2 = (EPS_vpid_eng_t) {15500,0,0};
    // 0b1110_1111_1110_0001 = 61409 the bit string for batter pack bitfield
    case2.system_eps_battery_datatype_struct = (EPS_battery_pack_datatype_eng_t) {vpid_case2, 61409 ,{0,0,0,0},{100,20,320}};
    case2.system_eps_conditioning_channel_info_each_channel[0] = (EPS_conditioning_channel_datatype_eng_t) {vpid_case2, 50,1,300,1000};
    case2.system_eps_conditioning_channel_info_each_channel[1] = (EPS_conditioning_channel_datatype_eng_t) {vpid_case2, 4,500,1000,1000};
    case2.system_eps_conditioning_channel_info_each_channel[2] = (EPS_conditioning_channel_datatype_eng_t) {vpid_case2, 6,8000,1000,1000};
    case2.system_eps_conditioning_channel_info_each_channel[3] = (EPS_conditioning_channel_datatype_eng_t) {vpid_case2, 1,5,1000,1000};

    SYS_TEMP_pack_to_system_thermal_info(&case2 , &result, err_case_2);
    TEST_ASSERT(result.system_OBC_temperature_cC == 1000);
    TEST_ASSERT(result.system_ANT_temperature_i2c_bus_A_cC == 5995);
    TEST_ASSERT(result.system_ANT_temperature_i2c_bus_B_cC == 4513);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[0] == 50);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[1] == 2000);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[2] == 48000);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[3] == 5);
    TEST_ASSERT(result.system_eps_battery_heater_status_bit == 0);
    TEST_ASSERT(result.system_eps_battery_percent == 87.50);
    TEST_ASSERT(result.system_eps_battery_each_sensor_temperature_cC[0]==100);
    TEST_ASSERT(result.system_eps_battery_each_sensor_temperature_cC[1]==20);
    TEST_ASSERT(result.system_eps_battery_each_sensor_temperature_cC[2]==320);

    //Case 3: Error on ANT B and PCU
    uint8_t err_case_3 = 0;
    err_case_3 |= SYS_TEMP_ANT_B_STATUS;
    err_case_3 |= SYS_TEMP_PCU_STATUS;

    SYS_TEMP_pack_to_system_thermal_info(&case1 , &result, err_case_3);

    TEST_ASSERT(result.system_OBC_temperature_cC == 5000);
    TEST_ASSERT(result.system_ANT_temperature_i2c_bus_A_cC == 4513);
    TEST_ASSERT(result.system_ANT_temperature_i2c_bus_B_cC == -99999);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[0] == -99999);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[1] == -99999);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[2] == -99999);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[3] == -99999);
    TEST_ASSERT(result.system_eps_battery_heater_status_bit == 1);
    TEST_ASSERT(result.system_eps_battery_percent == 75);
    TEST_ASSERT(result.system_eps_battery_each_sensor_temperature_cC[0]==400);
    TEST_ASSERT(result.system_eps_battery_each_sensor_temperature_cC[1]==0);
    TEST_ASSERT(result.system_eps_battery_each_sensor_temperature_cC[2]==550);

    //Case 4: Error on PBU
    uint8_t err_case_4 = 0;
    err_case_4 |= SYS_TEMP_ANT_A_STATUS;
    err_case_4 |= SYS_TEMP_PBU_STATUS;

    SYS_TEMP_pack_to_system_thermal_info(&case1 , &result, err_case_4);

    TEST_ASSERT(result.system_OBC_temperature_cC == 5000);
    TEST_ASSERT(result.system_ANT_temperature_i2c_bus_A_cC == -99999);
    TEST_ASSERT(result.system_ANT_temperature_i2c_bus_B_cC == 4513);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[0] == 1000000);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[1] == 100000);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[2] == 10000);
    TEST_ASSERT(result.system_solar_panel_power_generation_mW[3] == 5);
    TEST_ASSERT(result.system_eps_battery_heater_status_bit == 9999);
    TEST_ASSERT(result.system_eps_battery_percent == -99999);
    TEST_ASSERT(result.system_eps_battery_each_sensor_temperature_cC[0]==-9999);
    TEST_ASSERT(result.system_eps_battery_each_sensor_temperature_cC[1]==-9999);
    TEST_ASSERT(result.system_eps_battery_each_sensor_temperature_cC[2]==-9999);

    return 0;
}
