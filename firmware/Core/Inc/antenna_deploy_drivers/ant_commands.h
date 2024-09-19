#ifndef __INCLUDE_GUARD_ANT_COMMANDS_H__
#define __INCLUDE_GUARD_ANT_COMMANDS_H__

#include <stdint.h>


struct Antenna_deployment_status {
    int antenna_1_deployed;
    int antenna_1_deployment_time_limit_reached;
    int antenna_1_deployment_system_active;
    int antenna_2_deployed;
    int antenna_2_deployment_time_limit_reached;
    int antenna_2_deployment_system_active;
    int antenna_3_deployed;
    int antenna_3_deployment_time_limit_reached;
    int antenna_3_deployment_system_active;
    int antenna_4_deployed;
    int antenna_4_deployment_time_limit_reached;
    int antenna_4_deployment_system_active;
    int independent_burn;
    int ignoring_deployment_switches;
    int antenna_system_armed;
};

uint8_t ANT_CMD_reset();
uint8_t ANT_CMD_arm_antenna_system();
uint8_t ANT_CMD_disarm_antenna_system();
uint8_t ANT_CMD_deploy_antenna(uint8_t antenna, uint8_t activation_time_seconds);
uint8_t ANT_CMD_start_automated_sequential_deployment(uint8_t activation_time_seconds);
uint8_t ANT_CMD_deploy_antenna_with_override(uint8_t antenna, uint8_t activation_time_seconds);
uint8_t ANT_CMD_cancel_deployment_system_activation();
uint8_t ANT_CMD_measure_temp(uint16_t *result);
uint8_t ANT_CMD_report_deployment_status(struct Antenna_deployment_status *response);
uint8_t ANT_CMD_report_antenna_deployment_activation_count(uint8_t antenna, uint8_t *response);
uint8_t ANT_CMD_report_antenna_deployment_activation_time(uint8_t antenna, uint16_t *result);



    
#endif /* __INCLUDE_GUARD_ANT_COMMANDS_H__ */