
#ifndef INCLUDE_GUARD__RTOS_TASK_HELPERS_H__
#define INCLUDE_GUARD__RTOS_TASK_HELPERS_H__

#include "cmsis_os.h"

/// @brief Register all tasks in an array to track their worst/highest stack usage.
typedef struct {
    osThreadId_t *task_handle;
    const osThreadAttr_t *task_attribute;

    /// @brief The lowest amount of stack space remaining in bytes, as of the last warning log call.
    uint32_t lowest_stack_bytes_remaining;
} FREERTOS_task_info_struct_t;

// This function shall be called at the start of each task.
void TASK_HELP_start_of_task(void);


#endif // INCLUDE_GUARD__RTOS_TASK_HELPERS_H__
