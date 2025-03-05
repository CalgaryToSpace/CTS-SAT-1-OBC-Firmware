#include "rtos_tasks/rtos_task_helpers.h"
#include "debug_tools/debug_uart.h"
#include "cmsis_os.h"

void TASK_service_write_mpi_data(void *argument) {
    TASK_HELP_start_of_task();
    
    while(1) {
        DEBUG_uart_print_str("MPI_Thread_Running!\n");
        osDelay(60000);    
    }
}