#ifndef INCLUDE_GUARD__OBC_INTERNAL_DRIVERS_
#define INCLUDE_GUARD__OBC_INTERNAL_DRIVERS_

typedef enum obc_rbf_state_enum_t{
    OBC_RBF_STATE_FLYING = 1,
    OBC_RBF_STATE_BENCH = 0,
} obc_rbf_state_enum_t;

obc_rbf_state_enum_t obc_get_rbf_state();

#endif // INCLUDE_GUARD__OBC_INTERNAL_DRIVERS_