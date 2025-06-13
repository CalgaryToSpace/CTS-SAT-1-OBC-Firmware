#ifndef INCLUDE_GUARD__RTOS_BULK_DOWNLINK_TASK_H
#define INCLUDE_GUARD__RTOS_BULK_DOWNLINK_TASK_H

#include <stdint.h>

extern uint32_t COMMS_bulk_downlink_delay_per_packet_ms;

void TASK_bulk_downlink(void *argument);

#endif // INCLUDE_GUARD__RTOS_BULK_DOWNLINK_TASK_H
