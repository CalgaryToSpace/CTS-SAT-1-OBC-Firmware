
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
#include "system/obc_internal_drivers.h"

#include "cmsis_os.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

/// @brief The period of the heartbeat task, in milliseconds. 0 to disable.
uint32_t TASK_heartbeat_period_ms = 10990;

char TASK_heartbeat_timing_str[128] = {0};

static void set_heartbeat_led(uint8_t enabled) {
    OBC_set_external_led(enabled);
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

void TASK_execute_telecommands(void *argument) {
    TASK_HELP_start_of_task();

    // Note: Must not be less than 200ms since last pet.
    // 250ms is a good period, because it takes 208 ms to uplink a max-sized (250-byte) telecommand
    // at 9600 baud. Therefore, period=250ms makes it so that telecommands will be executed as fast
    // as they are uplinked.
    // Adding a short path nested loop that continues executing commands while they're available
    // is unnecessary. It is helpful to have a slight delay between commands to prevent flooding
    // the system with commands (hence the watchdog minimum interval being 200ms).
    // Conclusion: All parts of the system can handle 4 commands per second.
    const uint32_t task_period_for_watchdog_pet_ms = 250;

    // Cannot pet the watchdog too quickly on boot.
    osDelay(task_period_for_watchdog_pet_ms * 2);

    while (1) {
        // DEBUG_uart_print_str("TASK_execute_telecommands -> top of while(1)\n");
        // Pet the watchdog. Must be pet every 16 seconds. Must be >= 200ms since last pet.
        HAL_IWDG_Refresh(&hiwdg);

        // Get the next telecommand to execute.
        const int16_t next_tcmd_slot = TCMD_get_next_tcmd_agenda_slot_to_execute();
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
