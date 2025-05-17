#include "rtos_tasks/rtos_task_helpers.h"
#include "mpi/mpi_command_handling.h"
#include "littlefs/littlefs_helper.h"
#include "debug_tools/debug_uart.h"
#include "cmsis_os.h"
#include "log/log.h"
#include "uart_handler/uart_handler.h"

#include <stdio.h>


void TASK_service_write_mpi_data(void *argument) {
    TASK_HELP_start_of_task();
    osDelay(5000);

    while(1) {
        // If median buffer contains data to write
        if (MPI_buffer_state == MPI_MEMORY_WRITE_STATUS_PENDING) {
            // Store the current time for this iteration
            const uint32_t start_time = HAL_GetTick();

            // Mount LittleFS to memory (Shouldn't generally happen)
            if (!LFS_is_lfs_mounted) {
                const int8_t mount_result = LFS_mount();
                LOG_message(
                    LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_SINK_ALL, 
                    "MPI Task: Had to mount LFS. LFS_mount->%d. Took %lu ms",
                    mount_result,
                    HAL_GetTick() - start_time
                );
            }

            // Write data to file
            const lfs_ssize_t write_result = lfs_file_write(
                &LFS_filesystem, &MPI_science_data_file_pointer,
                (uint8_t*)MPI_active_data_median_buffer,
                MPI_active_data_median_buffer_len
            );
            if (write_result < 0) {
                LOG_message(
                    LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "MPI Task: Error writing to file: %ld", write_result
                );
            }
            LOG_message(
                LOG_SYSTEM_MPI, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
                "MPI Task: Successfully wrote %ld bytes to file",
                write_result
            );
            MPI_buffer_state = MPI_MEMORY_WRITE_STATUS_READY;

            // Log the time it took to write the data
            LOG_message(
                LOG_SYSTEM_MPI, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
                "MPI Task: Write Time: %lu", HAL_GetTick() - start_time
            );
        }
        
        // Do a short delay to allow recording to start right away once enabled.
        osDelay(100);
    }
}