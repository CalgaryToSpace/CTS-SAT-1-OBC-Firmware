#ifndef INCLUDE_GUARD__OBC_INTERNAL_DRIVERS_
#define INCLUDE_GUARD__OBC_INTERNAL_DRIVERS_

#include <stdint.h>

typedef enum {
    OBC_RBF_STATE_FLYING = 1,
    OBC_RBF_STATE_BENCH = 0,
} OBC_rbf_state_enum_t;

OBC_rbf_state_enum_t OBC_get_rbf_state();


void OBC_set_external_led(uint8_t enabled);

#endif // INCLUDE_GUARD__OBC_INTERNAL_DRIVERS_