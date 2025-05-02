#ifndef INCLUDE_GUARD__OBC_INTERNAL_DRIVERS_
#define INCLUDE_GUARD__OBC_INTERNAL_DRIVERS_

typedef enum {
    OBC_RBF_STATE_FLYING = 1,
    OBC_RBF_STATE_BENCH = 0,
} OBC_rbf_state_enum_t;

OBC_rbf_state_enum_t OBC_get_rbf_state();

#endif // INCLUDE_GUARD__OBC_INTERNAL_DRIVERS_