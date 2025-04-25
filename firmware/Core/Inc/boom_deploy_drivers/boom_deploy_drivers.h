#ifndef INCLUDE_GUARD__BOOM_DEPLOY_DRIVERS_H
#define INCLUDE_GUARD__BOOM_DEPLOY_DRIVERS_H

#include <stdint.h>

uint8_t BOOM_set_burn_enabled(uint8_t boom_channel_num, uint8_t enabled);

void BOOM_disable_all_burns(void);

#endif /* INCLUDE_GUARD__BOOM_DEPLOY_DRIVERS_H */
