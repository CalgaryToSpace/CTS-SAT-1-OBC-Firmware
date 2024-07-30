
#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"
#include "transforms/arrays.h"
#include "unit_tests/unit_test_executor.h"
#include "timekeeping/timekeeping.h"

#include "telecommands/freertos_telecommand_defs.h"
#include "debug_tools/debug_uart.h"


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <FreeRTOS.h>
#include <task.h>
#include <portmacro.h>

/// @brief Converts a FreeRTOS TaskState enum to a string representation.
/// @param state Input TaskState enum
/// @return A pointer to a C-string representing the input TaskState enum (statically-allocated).
const char* freertos_eTaskState_to_str(eTaskState state) {
    switch (state) {
        case eRunning:    return "eRunning";
        case eReady:      return "eReady";
        case eBlocked:    return "eBlocked";
        case eSuspended:  return "eSuspended";
        case eDeleted:    return "eDeleted";
        case eInvalid:    return "eInvalid";
        default:          return "Other";
    }
}


/// @brief Telecommand that return metadata regarding Tasks from FreeRTOS
/// @param args_str No arguments expected
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 if successful, >0 if an error occurred (but hello_world can't return an error)
uint8_t TCMDEXEC_freetos_list_tasks_jsonl(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    const UBaseType_t number_of_tasks = uxTaskGetNumberOfTasks();
    uint32_t total_run_time;
    TaskStatus_t task_statuses[number_of_tasks];

    if (uxTaskGetSystemState(task_statuses, number_of_tasks, &total_run_time) == 0) {
        DEBUG_uart_print_str("Error: TCMDEXEC_freetos_list_tasks_jsonl: uxTaskGetSystemState failed.\n");
        return 1;
    }

    for (UBaseType_t x = 0; x < number_of_tasks; x++) {
        // Get the task state for the x-th task
        char message_buffer[256];
        snprintf(
            message_buffer, sizeof(message_buffer),
            "{\"task_name\":\"%s\",\"state\":\"%s\",\"priority\":%lu,\"stack_min_remaining_bytes\":%u,\"runtime\":%lu}\n",
            task_statuses[x].pcTaskName,
            freertos_eTaskState_to_str(task_statuses[x].eCurrentState),
            task_statuses[x].uxCurrentPriority,
            // `usStackHighWaterMark`: The minimum amount of stack space that has remained for the task since the task was created.
            // The closer this value is to zero the closer the task has come to overflowing its stack.
            task_statuses[x].usStackHighWaterMark,
            task_statuses[x].ulRunTimeCounter
        );
          
        DEBUG_uart_print_str(message_buffer);
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "{\"number_of_tasks\":%lu,\"total_run_time\":%lu}", number_of_tasks, total_run_time
    );
    return 0;
}