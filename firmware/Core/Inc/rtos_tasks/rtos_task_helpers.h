
#ifndef __INCLUDE_GUARD__RTOS_TASK_HELPERS_H__
#define __INCLUDE_GUARD__RTOS_TASK_HELPERS_H__

#include "cmsis_os.h"
typedef struct {
    osThreadId_t *task_handle;
    const osThreadAttr_t *task_attribute;
    uint32_t lowest_highstack_watermark_bytes;
} Task_Info_t;

// This function shall be called at the start of each task.
void TASK_HELP_start_of_task(void);


#endif // __INCLUDE_GUARD__RTOS_TASK_HELPERS_H__
