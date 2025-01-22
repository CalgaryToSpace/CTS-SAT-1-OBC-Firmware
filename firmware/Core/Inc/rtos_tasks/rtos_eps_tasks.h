#ifndef __INCLUDE_GUARD_RTOS_EPS_TASKS_H__
#define __INCLUDE_GUARD_RTOS_EPS_TASKS_H__

extern uint64_t watchdog_timer, EPS_monitor_timer;

void TASK_service_eps_watchdog(void *argument);
void TASK_time_sync(void *argument);
#endif // __INCLUDE_GUARD_RTOS_EPS_TASKS_H__`