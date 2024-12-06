#ifndef __INCLUDE_GUARD__ANTENNA_DEPLOY_STARTUP_H__
#define __INCLUDE_GUARD__ANTENNA_DEPLOY_STARTUP_H__
#include <stdint.h>

int16_t START_deploy_antenna();
int16_t START_deploy_antenna_if_sufficiently_charged();
int16_t START_read_config_and_deploy_antenna_accordingly();
#endif  // __INCLUDE_GUARD__ANTENNA_DEPLOY_STARTUP_H__