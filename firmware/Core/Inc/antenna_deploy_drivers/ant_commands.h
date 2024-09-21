#ifndef __INCLUDE_GUARD_ANT_COMMANDS_H__
#define __INCLUDE_GUARD_ANT_COMMANDS_H__

#include <stdint.h>


struct Antenna_deployment_status {
    uint8_t antenna_1_deployed;
    uint8_t antenna_1_deployment_time_limit_reached;
    uint8_t antenna_1_deployment_system_active;
    uint8_t antenna_2_deployed;
    uint8_t antenna_2_deployment_time_limit_reached;
    uint8_t antenna_2_deployment_system_active;
    uint8_t antenna_3_deployed;
    uint8_t antenna_3_deployment_time_limit_reached;
    uint8_t antenna_3_deployment_system_active;
    uint8_t antenna_4_deployed;
    uint8_t antenna_4_deployment_time_limit_reached;
    uint8_t antenna_4_deployment_system_active;
    uint8_t independent_burn;
    uint8_t ignoring_deployment_switches;
    uint8_t antenna_system_armed;
};

uint8_t ANT_CMD_reset();
uint8_t ANT_CMD_arm_antenna_system();
uint8_t ANT_CMD_disarm_antenna_system();
uint8_t ANT_CMD_deploy_antenna(uint8_t antenna, uint8_t activation_time_seconds);
uint8_t ANT_CMD_start_automated_sequential_deployment(uint8_t activation_time_seconds);
uint8_t ANT_CMD_deploy_antenna_with_override(uint8_t antenna, uint8_t activation_time_seconds);
uint8_t ANT_CMD_cancel_deployment_system_activation();
uint8_t ANT_CMD_measure_temp(uint16_t *result);
int32_t ANT_CMD_convert_temp_measurement_to_centi_degree_celsius(uint16_t measurement);
uint8_t ANT_CMD_report_deployment_status(struct Antenna_deployment_status *response);
uint8_t ANT_CMD_report_antenna_deployment_activation_count(uint8_t antenna, uint8_t *response);
uint8_t ANT_CMD_report_antenna_deployment_activation_time(uint8_t antenna, uint16_t *result);



    
#endif /* __INCLUDE_GUARD_ANT_COMMANDS_H__ */