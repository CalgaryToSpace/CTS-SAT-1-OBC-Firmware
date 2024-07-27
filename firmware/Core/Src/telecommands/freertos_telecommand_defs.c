
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

uint8_t TCMDEXEC_retrieve_freertos_metadata(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
                            
    TaskStatus_t *pxTaskStatusArray;
    UBaseType_t uxArraySize, x;
    unsigned long ulTotalRunTime;

    uxArraySize = uxTaskGetNumberOfTasks();
    pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    if (pxTaskStatusArray != NULL) {

        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);
        DEBUG_uart_print_str("[");

        for (x = 0; x < uxArraySize; x++) {
            char message_buffer[256];
            int len = snprintf(message_buffer, sizeof(message_buffer),
                               "{\"TaskName\":\"%s\",\"State\":\"%s\",\"Priority\":%lu,\"Stack\":%u}%s",
                               pxTaskStatusArray[x].pcTaskName,
                               pxTaskStatusArray[x].eCurrentState == eRunning ? "RUN" :
                               pxTaskStatusArray[x].eCurrentState == eReady ? "RDY" :
                               pxTaskStatusArray[x].eCurrentState == eBlocked ? "BLK" :
                               pxTaskStatusArray[x].eCurrentState == eSuspended ? "SUSP" :
                               pxTaskStatusArray[x].eCurrentState == eDeleted ? "DEL" : "UNKN",
                               pxTaskStatusArray[x].uxCurrentPriority,
                               pxTaskStatusArray[x].usStackHighWaterMark,
                               x < uxArraySize - 1 ? "," : "");
            if (len > 0) {
                DEBUG_uart_print_str(message_buffer);
            }
        }

        DEBUG_uart_print_str("]");
        vPortFree(pxTaskStatusArray);
        
        } else {
            DEBUG_uart_print_str("Failed to allocate memory for task status array.");
            return 1;
            }

    return 0;
}