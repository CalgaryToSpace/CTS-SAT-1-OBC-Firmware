#include "rtos_tasks/rtos_task_helpers.h"
#include "mpi/mpi_command_handling.h"
#include "littlefs/littlefs_helper.h"
#include "debug_tools/debug_uart.h"
#include "cmsis_os.h"
#include "log/log.h"
#include "uart_handler/uart_handler.h"

#include <stdio.h>

uint32_t total_start_time;
uint8_t time_stored = 0;

void TASK_service_write_mpi_data(void *argument) {
    TASK_HELP_start_of_task();
    osDelay(1000); // FIXME: What should be a good delay here?

    while(1) {

        // If median buffer contains data to write
        if (MPI_buffer_state == MPI_MEMORY_WRITE_STATUS_PENDING) {

            // Store the current time if not already done
            if (time_stored == 0) {
                total_start_time = HAL_GetTick();
                time_stored = 1;
            }

            // Store the current time for this iteration
            uint32_t start_time = HAL_GetTick();

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

        // If user has sent disable active mode telecommand 
        else if (MPI_science_file_can_close == 1 && MPI_current_uart_rx_mode != MPI_RX_MODE_SENSING_MODE) {

            // Close the File, the storage is not updated until the file is closed successfully
            const int8_t close_result = lfs_file_close(&LFS_filesystem, &MPI_science_data_file_pointer);
            if (close_result < 0) {
                LOG_message(
                    LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "MPI Task: Error closing file: %d", close_result
                );
            }

            // Log that file has been successfully closed 
            LOG_message(
                LOG_SYSTEM_MPI, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
                "MPI Task: File closed successfully"
            );

            // Get the file size before closing the file
            const lfs_ssize_t file_size = lfs_file_size(&LFS_filesystem, &MPI_science_data_file_pointer);

            if (file_size < 0) {
                LOG_message(
                    LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "MPI Task: Error getting file size: %ld", file_size
                );
            }
            
            // Log MPI science data stats  
            LOG_message(
                LOG_SYSTEM_MPI, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
                "\nData Successfully Stored: %ld bytes\nData Lost: %lu bytes\nTotal Time Taken: %lu ms",
                file_size, MPI_science_data_bytes_lost, HAL_GetTick() - total_start_time 
            );

            MPI_science_file_can_close = 0;
            MPI_science_data_file_is_open = 0;
            MPI_science_data_bytes_lost = 0;
            time_stored = 0;
        }

        // FIXME: Are these delays fine? Should the be less/more?
        // If MPI_UART in sensing mode, do a shorter delay, otherwise 1s delay
        if (MPI_current_uart_rx_mode == MPI_RX_MODE_SENSING_MODE) {
            osDelay(100);
        }
        else {
            osDelay(1000);
        }
    }
}