#ifndef __INCLUDE_GUARD_ANT_COMMANDS_H__
#define __INCLUDE_GUARD_ANT_COMMANDS_H__

#include <stdint.h>

/*-----------------------------COMMAND VARIABLES-----------------------------*/
// All commands in this section refer to the "ISIS.ANTS.UM.001" datasheet by ISISpace
static const uint8_t ANT_CMD_RESET = 0xAA;
static const uint8_t ANT_CMD_ARM_ANTENNA_SYSTEM = 0xAD; // Arm the antenna deploy system
static const uint8_t ANT_CMD_DISARM_ANTENNA_SYSTEM = 0xAC;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA1 = 0xA1; // Deploy antenna 1
static const uint8_t ANT_CMD_DEPLOY_ANTENNA2 = 0xA2;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA3 = 0xA3;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA4 = 0xA4;
static const uint8_t ANT_CMD_DEPLOY_ALL_ANTENNAS_SEQ = 0xA5;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA1_OVERRIDE = 0xBA;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA2_OVERRIDE = 0xBB;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA3_OVERRIDE = 0xBC;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA4_OVERRIDE = 0xBD;
static const uint8_t ANT_CMD_CANCEL_DEPLOYMENT = 0xA9;
static const uint8_t ANT_CMD_MEASURE_TEMP = 0xC0; // Measure the temperature of the antenna controller system
static const uint8_t ANT_CMD_REPORT_DEPLOYMENT_STATUS = 0xC3;
static const uint8_t ANT_CMD_REPORT_ANT1_DEPLOYMENT_COUNT = 0xB0;
static const uint8_t ANT_CMD_REPORT_ANT2_DEPLOYMENT_COUNT = 0xB1;
static const uint8_t ANT_CMD_REPORT_ANT3_DEPLOYMENT_COUNT = 0xB2;
static const uint8_t ANT_CMD_REPORT_ANT4_DEPLOYMENT_COUNT = 0xB3;
static const uint8_t ANT_CMD_REPORT_ANT1_DEPLOYMENT_SYS_ACTIVATION_TIME= 0xB4;
static const uint8_t ANT_CMD_REPORT_ANT2_DEPLOYMENT_SYS_ACTIVATION_TIME= 0xB5;
static const uint8_t ANT_CMD_REPORT_ANT3_DEPLOYMENT_SYS_ACTIVATION_TIME= 0xB6;
static const uint8_t ANT_CMD_REPORT_ANT4_DEPLOYMENT_SYS_ACTIVATION_TIME= 0xB7;

uint8_t ANT_CMD_reset();
uint8_t ANT_CMD_arm_antenna_system();
uint8_t ANT_CMD_disarm_antenna_system();
uint8_t ANT_CMD_deploy_antenna(uint8_t antenna, uint8_t activation_time_seconds);
uint8_t ANT_CMD_start_automated_sequential_deployment(uint8_t activation_time_seconds);
uint8_t ANT_CMD_deploy_antenna_with_override(uint8_t antenna, uint8_t activation_time_seconds);
uint8_t ANT_CMD_cancel_deployment_system_activation();
uint8_t ANT_CMD_measure_temp();
uint8_t ANT_CMD_report_deployment_status(uint8_t response[2]);
uint8_t ANT_CMD_report_antenna_deployment_activation_count(uint8_t antenna, uint8_t *response);
uint8_t ANT_CMD_report_antenna_deployment_activation_time(uint8_t antenna, uint16_t *result);


struct Antenna_deployment_status {
    int is_antenna_1_deployed;
    int is_antenna_2_deployed;
    int is_antenna_3_deployed;
    int is_antenna_4_deployed;
};
#endif /* __INCLUDE_GUARD_ANT_COMMANDS_H__ */