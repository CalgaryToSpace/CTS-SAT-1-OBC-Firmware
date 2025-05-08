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
    osDelay(1000);
    
    uint32_t start_time;

    while(1) {
        // Add a check that would see if there is data to be written regardless of RX Mode, we write
        if (MPI_buffer_state == MPI_MEMORY_WRITE_STATUS_PENDING && MPI_receive_prepared) {

            start_time = HAL_GetTick();
            if (!LFS_is_lfs_mounted) {
                LFS_mount();
                LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), 
                "Had to mount LFS (System wasn't prepared for MPI data)");
            }
            LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Mount Time: %lu", HAL_GetTick() - start_time);


            // Write data to file
            const lfs_ssize_t write_result = lfs_file_write(
                &LFS_filesystem, &MPI_science_data_file_pointer,
                (uint8_t*)MPI_active_data_median_buffer,
                MPI_active_data_median_buffer_len
            );

            if (write_result < 0)
            {
                LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Error writing to file: %ld", write_result);
            } else {
                LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Successfully Wrote 4096 bytes");
                MPI_buffer_state = MPI_MEMORY_WRITE_STATUS_READY;
            }
            LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Write Time: %lu", HAL_GetTick() - start_time);

        } else {
            if (MPI_receive_prepared == 2 && MPI_current_uart_rx_mode != MPI_RX_MODE_SENSING_MODE) {
                // Close the File, the storage is not updated until the file is closed successfully
                const int8_t close_result = lfs_file_close(&LFS_filesystem, &MPI_science_data_file_pointer);
                if (close_result < 0)
                {
                    LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Error closing file: %d", close_result);
                } 
                LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "File Successfully closed");
                MPI_receive_prepared = 0;
                MPI_science_data_file_is_open = 0;
            }
        }
        if (MPI_current_uart_rx_mode == MPI_RX_MODE_SENSING_MODE) {
            osDelay(100);
        } else {
            osDelay(1000);
        }
    }
}