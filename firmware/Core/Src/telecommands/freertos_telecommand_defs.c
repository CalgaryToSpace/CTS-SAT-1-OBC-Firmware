
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


/// @brief Telecommand that return metadata regarding Tasks from FreeRTOS
/// @param args_str No arguments expected
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 if successful, >0 if an error occurred (but hello_world can't return an error)
const char* freertos_current_state_enum_to_str(eTaskState state) {
    switch (state) {
        case eRunning:    return "RUN";
        case eReady:      return "RDY";
        case eBlocked:    return "BLK";
        case eSuspended:  return "SUSP";
        case eDeleted:    return "DEL";
        default:          return "UNKN";
    }
}

uint8_t TCMDEXEC_freertos_get_metadata(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
                            
    TaskStatus_t *px_task_status_array;
    UBaseType_t ux_array_size, x;
    unsigned long ul_total_run_time;

    ux_array_size = uxTaskGetNumberOfTasks();
    px_task_status_array = pvPortMalloc(ux_array_size * sizeof(TaskStatus_t));

    if (px_task_status_array == NULL) {
        DEBUG_uart_print_str("Failed to allocate memory for task status array.");
        return 1;
    }

    ux_array_size = uxTaskGetSystemState(px_task_status_array, ux_array_size, &ul_total_run_time);
    DEBUG_uart_print_str("[");

    for (x = 0; x < ux_array_size; x++) {
        char message_buffer[256];
        int len = snprintf(message_buffer, sizeof(message_buffer),
                            "{\"TaskName\":\"%s\",\"State\":\"%s\",\"Priority\":%lu,\"Stack\":%u}%s",
                            px_task_status_array[x].pcTaskName,
                            freertos_current_state_enum_to_str(px_task_status_array[x].eCurrentState),
                            px_task_status_array[x].uxCurrentPriority,
                            px_task_status_array[x].usStackHighWaterMark,
                            x < ux_array_size - 1 ? "," : "");
        if (len > 0) {
            DEBUG_uart_print_str(message_buffer);
        }
    }

    DEBUG_uart_print_str("]");
    vPortFree(px_task_status_array);
    
    return 0;
}