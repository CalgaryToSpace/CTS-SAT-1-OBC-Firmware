#ifndef __INCLUDE_GUARD_ANT_COMMANDS_H__
#define __INCLUDE_GUARD_ANT_COMMANDS_H__

#include <stdint.h>

/*-----------------------------COMMAND VARIABLES-----------------------------*/
// All commands in this section refer to the "ISIS.ANTS.UM.001" datasheet by ISISpace

static const uint8_t ANT_CMD_ARM_ANTENNA_SYSTEM = 0xAD; // Arm the antenna deploy system
static const uint8_t ANT_CMD_DEPLOY_ANTENNA1 = 0xA1; // Deploy antenna 1
static const uint8_t ANT_CMD_MEASURE_TEMP = 0xC0; // Measure the temperature of the antenna controller system

uint8_t ANT_CMD_arm_antenna_system();
uint8_t ANT_CMD_deploy_antenna1(uint8_t ativation_time_seconds);
uint8_t ANT_CMD_measure_temp();

#endif /* __INCLUDE_GUARD_ANT_COMMANDS_H__ */