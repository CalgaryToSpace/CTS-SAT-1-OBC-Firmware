#ifndef INCLUDE_GUARD__RTOS_BOOTUP_OPERATION_FSM_TASK_H
#define INCLUDE_GUARD__RTOS_BOOTUP_OPERATION_FSM_TASK_H

#include <stdint.h>

extern uint32_t COMMS_uptime_to_start_ant_deployment_sec;

typedef enum {
    // Entry point. Booted, and awaiting a trigger to deployment. Emulate a 60-minute countdown to antenna deployment.
    CTS1_OPERATION_STATE_BOOTED_AND_WAITING,

    // Intermediate state. Very short-lived nominally. Deploy deployable antennas here.
    CTS1_OPERATION_STATE_DEPLOYING,

    // Normal operation state, with radio transmission enabled (e.g., flying through space).
    CTS1_OPERATION_STATE_NOMINAL_WITH_RADIO_TX,

    // Normal operation state, with radio transmission disabled (e.g., during bench testing).
    CTS1_OPERATION_STATE_NOMINAL_WITHOUT_RADIO_TX
} CTS1_operation_state_enum_t;

extern CTS1_operation_state_enum_t CTS1_operation_state;


char* CTS1_operation_state_enum_TO_str(CTS1_operation_state_enum_t state);


void TASK_bootup_operation_fsm(void *argument);

#endif // INCLUDE_GUARD__RTOS_BOOTUP_OPERATION_FSM_TASK_H
