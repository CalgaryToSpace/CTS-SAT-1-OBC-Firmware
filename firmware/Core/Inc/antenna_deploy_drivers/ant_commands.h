#ifndef INCLUDE_GUARD_ANT_COMMANDS_H__
#define INCLUDE_GUARD_ANT_COMMANDS_H__

#include <stdint.h>
#include "antenna_deploy_drivers/ant_internal_drivers.h"


/// @brief Duration to wait after enabling power to the antenna deployment system before sending commands.
/// @note 100ms seems too short. 800ms seems to be a good value.
static const uint32_t ANT_POWER_ON_BOOTUP_DURATION_MS = 800;

typedef struct {
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
} ANT_deployment_status_struct_t;

uint8_t ANT_CMD_reset(enum ANT_i2c_bus_mcu i2c_bus_mcu);
uint8_t ANT_CMD_arm_antenna_system(enum ANT_i2c_bus_mcu i2c_bus_mcu);
uint8_t ANT_CMD_disarm_antenna_system(enum ANT_i2c_bus_mcu i2c_bus_mcu);
uint8_t ANT_CMD_deploy_antenna(enum ANT_i2c_bus_mcu i2c_bus_mcu, uint8_t antenna, uint8_t activation_time_seconds);
uint8_t ANT_CMD_start_automated_sequential_deployment(enum ANT_i2c_bus_mcu i2c_bus_mcu, uint8_t activation_time_seconds);
uint8_t ANT_CMD_deploy_antenna_with_override(enum ANT_i2c_bus_mcu i2c_bus_mcu, uint8_t antenna, uint8_t activation_time_seconds);
uint8_t ANT_CMD_cancel_deployment_system_activation(enum ANT_i2c_bus_mcu i2c_bus_mcu);
uint8_t ANT_CMD_measure_temp(enum ANT_i2c_bus_mcu i2c_bus_mcu, uint16_t *result);
int16_t ANT_convert_raw_temp_to_cCelsius(uint16_t measurement);
uint8_t ANT_CMD_report_deployment_status(enum ANT_i2c_bus_mcu i2c_bus_mcu, ANT_deployment_status_struct_t *response);
uint8_t ANT_CMD_report_antenna_deployment_activation_count(enum ANT_i2c_bus_mcu i2c_bus_mcu, uint8_t antenna, uint8_t *response);
uint8_t ANT_CMD_get_antenna_deployment_activation_time(enum ANT_i2c_bus_mcu i2c_bus_mcu, uint8_t antenna, uint32_t *result);



    
#endif /* INCLUDE_GUARD_ANT_COMMANDS_H__ */