
#include "main.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "rtos_tasks/rtos_tasks.h"
#include "telecommand_exec/telecommand_parser.h"
#include "telecommand_exec/telecommand_executor.h"
#include "debug_tools/debug_uart.h"
#include "timekeeping/timekeeping.h"
#include "uart_handler/uart_handler.h"
#include "transforms/arrays.h"
#include "stm32/stm32_reboot_reason.h"
#include "log/log.h"
#include "config/configuration.h"
#include "eps_drivers/eps_commands.h"
#include "littlefs/littlefs_helper.h"
#include "camera/camera_init.h"

#include "cmsis_os.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

/// @brief The period of the heartbeat task, in milliseconds. 0 to disable.
uint32_t TASK_heartbeat_period_ms = 10990;

char TASK_heartbeat_timing_str[128] = {0};

void set_heartbeat_led(uint8_t enabled) {
    HAL_GPIO_WritePin(PIN_LED_DEVKIT_LD2_GPIO_Port, PIN_LED_DEVKIT_LD2_Pin, enabled);
    HAL_GPIO_WritePin(PIN_LED_GP2_OUT_GPIO_Port, PIN_LED_GP2_OUT_Pin, enabled);
}

void TASK_DEBUG_print_heartbeat(void *argument) {
    TASK_HELP_start_of_task();

    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "TASK_DEBUG_print_heartbeat() -> started (booted)."
    );

    // Fetch the reset cause right upon boot so that it is logged for each boot immediately.
    const char* STM32_reset_cause_name = STM32_reset_cause_enum_to_str(STM32_get_reset_cause());
    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Reset reason: %s.", STM32_reset_cause_name
    );

    // Blink the LED a few times to show that the boot just happened.
    for (uint8_t i = 0; i < 12; i++) {
        set_heartbeat_led((i+1) % 2);
        HAL_Delay(100 + (i*25));
    }

    osDelay(TASK_heartbeat_period_ms > 0 ? TASK_heartbeat_period_ms : 1000);

    uint64_t unix_time_ms = 0;
    time_t seconds = 0;
    uint16_t ms = 0;
    struct tm *time_info;

    while (1) {
        if ((TASK_heartbeat_period_ms > 0)) {
            unix_time_ms = TIM_get_current_unix_epoch_time_ms();
            seconds = (time_t)(unix_time_ms/ 1000U);
            ms = unix_time_ms - 1000U * seconds;
            time_info = gmtime(&seconds);
            
            LOG_message(
                LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
                "Heartbeat: FrontierSat time: %d%02d%02dT%02d:%02d:%02d.%03u, Uptime: %lu ms\n",
                time_info->tm_year + 1900, time_info->tm_mon + 1, time_info->tm_mday,
                time_info->tm_hour, time_info->tm_min, time_info->tm_sec, ms,
                HAL_GetTick()
            );

            // TODO: Radio beacon here, probably.
        }
        set_heartbeat_led(1);
        osDelay(100); // Reminder: May take longer than 100ms.
        set_heartbeat_led(0);

        osDelay(TASK_heartbeat_period_ms > 0 ? TASK_heartbeat_period_ms : 1000);
    }
}


void TASK_handle_uart_telecommands(void *argument) {
    TASK_HELP_start_of_task();

    // CONFIGURATION PARAMETER
    uint32_t timeout_duration_ms = 100;

    char latest_tcmd[UART_telecommand_buffer_len];
    uint16_t latest_tcmd_len = 0;

    while (1) {
        // place the main delay at the top to avoid a "continue" statement skipping it
        osDelay(400);

        // DEBUG_uart_print_str("TASK_handle_uart_telecommands -> top of while(1)\n");

        memset(latest_tcmd, 0, UART_telecommand_buffer_len);
        latest_tcmd_len = 0; // 0 means no telecommand available

        // log the status
        // char msg[256];
        // snprintf(msg, sizeof(msg), "UART telecommand buffer: write_index=%d, last_time=%lums\n", UART_telecommand_buffer_write_idx, UART_telecommand_last_write_time_ms);
        // DEBUG_uart_print_str(msg);

        if ((HAL_GetTick() - UART_telecommand_last_write_time_ms > timeout_duration_ms) && (UART_telecommand_buffer_write_idx > 0)) {
            // Copy the buffer to the latest_tcmd buffer.
            latest_tcmd_len = UART_telecommand_buffer_write_idx;
            
            // MEMCPY, but with volatile-compatible casts.
            // Copy the whole buffer to ensure nulls get copied too.
            for (uint16_t i = 0; i < UART_telecommand_buffer_len; i++) {
                latest_tcmd[i] = (char) UART_telecommand_buffer[i];
            }

            // Set the null terminator at the end of the `latest_tcmd` str.
            latest_tcmd[latest_tcmd_len] = '\0';

            // Clear the buffer (memset to 0, but volatile-compatible) and reset the write pointer.
            for (uint16_t i = 0; i < UART_telecommand_buffer_len; i++) {
                UART_telecommand_buffer[i] = 0;
            }
            UART_telecommand_buffer_write_idx = 0;
            // TODO: could do it so that it only clears the part of the buffer which contains a command, to allow multiple commands per buffer
        }

        if (latest_tcmd_len == 0) {
            continue;
        }

        // DEBUG_uart_print_str("========================= UART Telecommand Received =========================\n");
        // DEBUG_uart_print_str(latest_tcmd);
        // DEBUG_uart_print_str("\n=========================\n");

        // Parse the telecommand
        TCMD_parsed_tcmd_to_execute_t parsed_tcmd;
        uint8_t parse_result = TCMD_parse_full_telecommand(
            latest_tcmd, TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd
        );
        if (parse_result != 0) {
            LOG_message(
                LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                "Error parsing telecommand: %u", parse_result
            );
            continue;
        }

        // Add the telecommand to the agenda (regardless of whether it's in the future).
        TCMD_add_tcmd_to_agenda(&parsed_tcmd);
    
    } /* End Task's Main Loop */
}

void TASK_execute_telecommands(void *argument) {
    TASK_HELP_start_of_task();

    // Note: Must not be less than 200ms since last pet.
    const uint32_t task_period_for_watchdog_pet_ms = 250;

    // Cannot pet the watchdog too quickly on boot.
    osDelay(task_period_for_watchdog_pet_ms * 2);

    while (1) {
        // DEBUG_uart_print_str("TASK_execute_telecommands -> top of while(1)\n");
        // Pet the watchdog. Must be pet every 16 seconds. Must be >= 200ms since last pet.
        HAL_IWDG_Refresh(&hiwdg);

        // Get the next telecommand to execute.
        int16_t next_tcmd_slot = TCMD_get_next_tcmd_agenda_slot_to_execute();
        if (next_tcmd_slot == -1) {
            // No telecommands to execute.
            // DEBUG_uart_print_str("No telecommands to execute.\n");
            osDelay(task_period_for_watchdog_pet_ms);
            continue;
        }

        // Execute the telecommand.
        char response_output_buf[TCMD_MAX_RESPONSE_BUFFER_LENGTH] = {0};
        TCMD_execute_telecommand_in_agenda(
            next_tcmd_slot,
            response_output_buf,
            sizeof(response_output_buf)
        );

        // Note: Short yield here only; execute all pending telecommands back-to-back.
        osDelay(task_period_for_watchdog_pet_ms);

    } /* End Task's Main Loop */
}

void TASK_monitor_freertos_memory(void *argument) {
    TASK_HELP_start_of_task();

    osDelay(12000); // Delay for 12 seconds to allow other tasks to start up.

    while (1) {
        // Place the main delay at the top to avoid a "continue" statement skipping it.
        osDelay(5000);

        for (uint16_t task_num = 0; task_num < FREERTOS_task_handles_array_size; task_num++) {
            if (FREERTOS_task_handles_array[task_num].task_handle == NULL) {
                continue; // Safety check. Should never happen.
            }
    
            // Dereferencing the task_handle pointer
            const osThreadId_t task_handle = *(FREERTOS_task_handles_array[task_num].task_handle);

            // Get the highstack watermark
            const uint32_t task_min_bytes_remaining = uxTaskGetStackHighWaterMark(task_handle) * 4;

            if (task_min_bytes_remaining < FREERTOS_task_handles_array[task_num].lowest_stack_bytes_remaining) {
                // If this is the new "lowest free space", update that value.
                FREERTOS_task_handles_array[task_num].lowest_stack_bytes_remaining = task_min_bytes_remaining;

                // Determine the threshold of the task
                const uint32_t task_threshold_bytes = (
                    FREERTOS_task_handles_array[task_num].task_attribute->stack_size
                    * CONFIG_freertos_min_remaining_stack_percent
                    / 100
                );
                
                // If this new "lowest free space" is below the threshold, warn the user.
                if (task_min_bytes_remaining < task_threshold_bytes) {
                    LOG_message(
                        LOG_SYSTEM_OBC, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                        "Warning: Task '%s' approached a stack overflow. Worst remaining stack size was: %lu bytes.",
                        pcTaskGetName(task_handle),
                        task_min_bytes_remaining
                    );
                }
            }
        }

    } /* End Task's Main Loop */
}

void TASK_camera_write_image(void *argument) {
	TASK_HELP_start_of_task();
	while (1) {
		osDelay(500);
        // if camera is receiving data start while loop
        if (UART_camera_is_expecting_data){
                // receive until response timed out

            // write file after half and complete callbacks
            if (camera_write_file){
                // debug string DELETE THIS AFTER TESTING
                DEBUG_uart_print_str("in write file\n");
                UART_camera_last_write_time_ms = HAL_GetTick();

                // Write data to file
                const lfs_ssize_t write_result = lfs_file_write(&LFS_filesystem, &file, UART_camera_rx_buf, SENTENCE_LEN*23);
                if (write_result < 0)
                {
                    LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Error writing to file: %s", file_name);
                }

                camera_write_file = 0;
            }


            // Timeout conditions
            if (UART_camera_buffer_write_idx == 0){
                // if write index is 0 and timeout occurs
                if ((HAL_GetTick() - UART_camera_rx_start_time_ms) > CAMERA_RX_TIMEOUT_DURATION_MS) {
                    // if last write time = 0 nothing was written, error 4
                    if (UART_camera_last_write_time_ms == 0){
                        cam_receive_error = true;
                        CAMERA_set_expecting_data(0);
                        continue; // Error code: Timeout waiting for first byte
                    }
                    else{
                        // otherwise there may be data in first half of buffer
                        // copy data and set write to 1 to write after exiting loop
                        for (uint16_t i = 0; i < UART_camera_buffer_len/2; i++){
                            UART_camera_rx_buf[i] = UART_camera_buffer[i];
                            UART_camera_buffer[i] = 0;
                        }
                        // PRINT FOR TESTING DELETE AFTER
                        DEBUG_uart_print_str("timeout write file 1\n");
                        camera_write_file = 1;
                        // finish receiving and break out of loop
                        CAMERA_set_expecting_data(0);
                        continue;
                    }
                }
            }

            // if write idx is not 0 then it must be 1
            else{
                // if write idx is 1 then there may be data in second half of buffer
                const uint32_t current_time = HAL_GetTick();
                if (
                    (current_time > UART_camera_last_write_time_ms) // obvious safety check
                    && ((current_time - UART_camera_last_write_time_ms) > CAMERA_RX_TIMEOUT_DURATION_MS))
                    {
                        // copy data from second half and set write file to true
                        for (uint16_t i = UART_camera_buffer_len/2; i < UART_camera_buffer_len; i++) {
                            UART_camera_rx_buf[i-UART_camera_buffer_len/2] = UART_camera_buffer[i];
                            UART_camera_buffer[i] = 0;
                        }
                        // PRINT FOR TESTING DELETE AFTER
                        DEBUG_uart_print_str("timeout write file 2\n"); 
                        camera_write_file = 1;
                        // finish receiving and break out of loop
                        CAMERA_set_expecting_data(0);
                        continue;
                    }
            }

        }

	} /* End Task's Main Loop */
}
