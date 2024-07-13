#include "main.h"

#include "antenna_deploy_drivers/ant_commands.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "stm32l4xx_hal_i2c.h"
#include "debug_tools/debug_i2c.h"

uint8_t ANT_CMD_measure_temp() {
    const uint8_t CC = 0xC0;

    
}