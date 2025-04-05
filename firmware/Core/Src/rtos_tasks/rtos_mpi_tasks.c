#include "rtos_tasks/rtos_task_helpers.h"
#include "mpi_command_handling.h"
#include "littlefs/littlefs_helper.h"
#include "debug_tools/debug_uart.h"
#include "cmsis_os.h"
#include "log/log.h"
#include <stdio.h>

void TASK_service_write_mpi_data(void *argument) {
    TASK_HELP_start_of_task();
    osDelay(30000);

    char file_name_buffer[60];
    uint8_t file_open = 0;
    lfs_file_t file;
    uint32_t start_time = 0;

    while(1) {
        // Add a check that would see if there is data to be written regardless of RX Mode, we write
        if (MPI_buffer_state == MPI_MEMORY_WRITE_STATUS_PENDING) {

            start_time = HAL_GetTick();
            if (!LFS_is_lfs_mounted) {
                LFS_mount();
            }

            // Create or Open a file with Write only flag if file not already open
            if (file_open == 0) {
                // Create file name
                sprintf(file_name_buffer, "mpi_active_data_file%lu", HAL_GetTick());

                const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name_buffer, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND, &LFS_file_cfg);
                if (open_result < 0) {
                    LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Error opening / creating file: %s", file_name_buffer);
                } else {
                    LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Opened/created file: %s", file_name_buffer);
                    file_open = 1;
                }
            }

            // Write data to file
            const lfs_ssize_t write_result = lfs_file_write(&LFS_filesystem, &file, MPI_active_data_median_buffer, MPI_active_data_median_buffer_len);
            if (write_result < 0)
            {
                LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Error writing to file: %s", file_name_buffer);
            } else {
                MPI_buffer_state = MPI_MEMORY_WRITE_STATUS_READY;
            }
            LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Wrote 4096 bytes in: %ldms", HAL_GetTick() - start_time);

        } else {
            if (file_open && MPI_current_uart_rx_mode != MPI_RX_MODE_SENSING_MODE) {
                // Close the File, the storage is not updated until the file is closed successfully
                const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
                if (close_result < 0)
                {
                    LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Error closing file: %s", file_name_buffer);
                } else {
                    LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "File: %s succesfully closed", file_name_buffer);
                }
                file_open = 0;
            }
        }
        if (MPI_current_uart_rx_mode == MPI_RX_MODE_SENSING_MODE) {
            osDelay(100);
        } else {
            osDelay(1000);
        }
    }
}