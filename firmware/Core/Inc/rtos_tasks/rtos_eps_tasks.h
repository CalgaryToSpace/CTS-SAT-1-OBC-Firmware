#ifndef INCLUDE_GUARD_RTOS_EPS_TASKS_H__
#define INCLUDE_GUARD_RTOS_EPS_TASKS_H__

extern uint64_t watchdog_timer, EPS_monitor_timer;

void TASK_service_eps_watchdog(void *argument);
void TASK_time_sync(void *argument);
#endif // INCLUDE_GUARD_RTOS_EPS_TASKS_H__`
