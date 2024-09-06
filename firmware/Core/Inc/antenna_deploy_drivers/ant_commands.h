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
static const uint8_t ANT_CMD_DEPLOY_ANTENNA1_OVERIDE = 0xBA;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA2_OVERIDE = 0xBB;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA3_OVERIDE = 0xBC;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA4_OVERIDE = 0xBD;
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

uint8_t ANT_CMD_arm_antenna_system();
uint8_t ANT_CMD_deploy_antenna1(uint8_t ativation_time_seconds);
uint8_t ANT_CMD_measure_temp();

#endif /* __INCLUDE_GUARD_ANT_COMMANDS_H__ */