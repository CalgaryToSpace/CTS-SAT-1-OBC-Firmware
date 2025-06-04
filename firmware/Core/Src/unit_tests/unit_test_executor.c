#include "unit_tests/unit_test_executor.h"
#include "unit_tests/unit_test_inventory.h"
#include "debug_tools/debug_uart.h"
#include "log/log.h"
#include "stm32/stm32_watchdog.h"

#include <string.h>
#include <stdio.h>

uint8_t TEST_run_all_unit_tests_and_log(char log_buffer[], uint16_t log_buffer_size) {
    uint16_t total_exec_count = 0;
    uint16_t total_pass_count = 0;
    uint16_t total_fail_count = 0;
    const uint32_t start_time_ms = HAL_GetTick();

    log_buffer[0] = '\0';

    uint32_t last_wdog_reset_time_ms = HAL_GetTick();

    for (uint8_t test_num = 0; test_num < TEST_definitions_count; test_num++) {
        const TEST_Function_Ptr test_function = TEST_definitions[test_num].test_func;
        const uint8_t result = test_function();

        LOG_message(
            LOG_SYSTEM_OBC,
            (result == 0) ? LOG_SEVERITY_DEBUG : LOG_SEVERITY_WARNING,
            LOG_SINK_ALL,
            "Test #%03d: %s (%s > %s)",
            test_num,
            (result == 0) ? "PASS ✅" : "FAIL ❌",
            TEST_definitions[test_num].test_file,
            TEST_definitions[test_num].test_func_name
        );
        
        total_exec_count++;
        if (result == 0) {
            total_pass_count++;
        }
        else {
            total_fail_count++;
        }

        // Hack to prevent the watchdog from resetting the STM32 during long test runs.
        if (HAL_GetTick() - last_wdog_reset_time_ms > 5000) {
            STM32_pet_watchdog();
            last_wdog_reset_time_ms = HAL_GetTick();
        }
    }
    const uint32_t end_time_ms = HAL_GetTick();

    snprintf(
        log_buffer,
        log_buffer_size,
        "{\"total_tests\": %d, \"passed\": %d, \"failed\": %d, \"duration_ms\": %lu}",
        total_exec_count,
        total_pass_count,
        total_fail_count,
        end_time_ms - start_time_ms
    );

    return total_fail_count == 0;
}
