// This file contains the implementation of the telecommand reception task for Debug/Umilical UART
// and KISS UART.

#include "main.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "rtos_tasks/rtos_tasks_rx_telecommands.h"
#include "telecommand_exec/telecommand_parser.h"
#include "telecommand_exec/telecommand_executor.h"
#include "uart_handler/uart_handler.h"
#include "transforms/arrays.h"
#include "log/log.h"
#include "debug_tools/debug_uart.h"

#include "cmsis_os.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

typedef enum {
    TCMD_CHECK_STATUS_NO_TCMD = 0,
    TCMD_CHECK_STATUS_TCMD_SCHEDULED = 1,
    TCMD_CHECK_STATUS_TCMD_INVALID_AND_DISCARDED = 2,
} TCMD_check_result_enum_t;

/// @brief Checks for an available telecommand in the UART buffer, and schedules it for execution.
/// @return 0 if no telecommand was available; 1 if a telecommand was found and executed.
/// @note This function is only used in this file.
/// @note Global extern variables used:
///     - UART_telecommand_buffer
///     - UART_telecommand_buffer_write_idx
///     - UART_telecommand_last_write_time_ms
static TCMD_check_result_enum_t check_for_and_handle_new_uart_tcmds() {
    const uint32_t timeout_duration_ms = 100; // CONFIGURATION PARAMETER

    char latest_tcmd[UART_telecommand_buffer_len];
    uint16_t latest_tcmd_len = 0;
    
    // Unnecessary, but good for safety.
    memset(latest_tcmd, 0, UART_telecommand_buffer_len);
    latest_tcmd_len = 0; // 0 means no telecommand available (checked later).

    // Debugging: Log the state of the UART buffer.
    // char msg[256];
    // snprintf(msg, sizeof(msg), "UART telecommand buffer: write_index=%d, last_time=%lums\n", UART_telecommand_buffer_write_idx, UART_telecommand_last_write_time_ms);
    // DEBUG_uart_print_str(msg);

    if (UART_telecommand_buffer_write_idx == 0) {
        // If we haven't received any telecommands, there is nothing to do here.
        return TCMD_CHECK_STATUS_NO_TCMD;
    }

    if (
        // If the end-of-message character was NOT received:
        GEN_get_index_of_substring_in_array(
            (char*) UART_telecommand_buffer,
            UART_telecommand_buffer_write_idx,
            "!"
        ) < 0
    ) {
        // If timed out on reception:
        if (HAL_GetTick() - UART_telecommand_last_write_time_ms > timeout_duration_ms) {
            // If the telecommand is not complete, discard it.

            // Null-terminate the string, then log it.
            UART_telecommand_buffer[UART_telecommand_buffer_write_idx] = '\0';
            LOG_message(
                LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                "Received incomplete telecommand. Discarding incomplete telecommand: '%s'",
                UART_telecommand_buffer
            );
            UART_telecommand_buffer_write_idx = 0;
            return TCMD_CHECK_STATUS_TCMD_INVALID_AND_DISCARDED;
        }
        else {
            // Keep waiting for a timeout or a complete telecommand.
            return TCMD_CHECK_STATUS_NO_TCMD;
        }
    }

    // Set `latest_tcmd_len` to the length of the telecommand.
    const int16_t idx_of_last_char_in_tcmd = GEN_get_index_of_substring_in_array(
        (char*) UART_telecommand_buffer,
        UART_telecommand_buffer_write_idx,
        "!"
    );
    if (idx_of_last_char_in_tcmd < 0) { // -1 indicates "string not found".
        // If the telecommand is not complete, discard it.
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_CRITICAL, LOG_SINK_ALL,
            "Should not happen: incomplete telecommand. Discarding incomplete telecommand."
        );
        UART_telecommand_buffer_write_idx = 0;
        return TCMD_CHECK_STATUS_TCMD_INVALID_AND_DISCARDED;
    }
    else {
        // End-of-message character found. Use the buffer up to that point.
        latest_tcmd_len = ((uint16_t) idx_of_last_char_in_tcmd) + 1;
    }
    
    // `memcpy()`, but volatile-compatible.
    for (uint16_t i = 0; i < latest_tcmd_len; i++) {
        latest_tcmd[i] = (char) UART_telecommand_buffer[i];
    }
    // Set the null terminator at the end of the `latest_tcmd` str.
    latest_tcmd[latest_tcmd_len] = '\0';

    // Shift or clear the buffer.
    if (latest_tcmd_len == UART_telecommand_buffer_write_idx) {
        // If the telecommand is the whole buffer, simply clear the buffer.
        UART_telecommand_buffer_write_idx = 0;
    }
    else {
        // Shift the buffer to the left by `latest_tcmd_len` bytes. Can't use memcpy.
        for (uint16_t i = latest_tcmd_len; i < UART_telecommand_buffer_write_idx; i++) {
            UART_telecommand_buffer[i - latest_tcmd_len] = (char) UART_telecommand_buffer[i];
        }
        // Update the write index.
        UART_telecommand_buffer_write_idx -= latest_tcmd_len;
    }

    if (latest_tcmd_len == 0) {
        return TCMD_CHECK_STATUS_NO_TCMD;
    }

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
        return TCMD_CHECK_STATUS_TCMD_INVALID_AND_DISCARDED;
    }

    // Add the telecommand to the agenda (regardless of whether it's in the future).
    TCMD_add_tcmd_to_agenda(&parsed_tcmd);

    return TCMD_CHECK_STATUS_TCMD_SCHEDULED;
}


void TASK_handle_uart_telecommands(void *argument) {
    TASK_HELP_start_of_task();

    while (1) {
        // Place the main delay at the top to avoid a "continue" statement skipping it.
        osDelay(400);

        // Continue checking and scheduling telecommands until there are no more to process.
        while (1) {
            const TCMD_check_result_enum_t check_result = check_for_and_handle_new_uart_tcmds();

            if (check_result == TCMD_CHECK_STATUS_NO_TCMD) {
                break;
            }
            else if (check_result == TCMD_CHECK_STATUS_TCMD_SCHEDULED) {
                // Nothing to do. Keep looping.
                continue;
            }
            else if (check_result == TCMD_CHECK_STATUS_TCMD_INVALID_AND_DISCARDED) {
                // Could either continue OR break here. Will break though, which just means that
                // rapid invalid telecommands will get parsed rather slowly.
                break;
            }
            else {
                // This should never happen.
                LOG_message(
                    LOG_SYSTEM_OBC, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "Error: check_for_and_handle_new_uart_tcmds() returned an invalid value: %u",
                    check_result
                );
            }
        }
    } /* End Task's Main Loop */
}


/// @brief Checks for an available telecommand in the UART buffer, and schedules it for execution.
/// @return 0 if no telecommand was available; 1 if a telecommand was found and executed.
/// @param csp_packet_array The CSP packet to read, starting with 4 bytes of CSP, and reading until null-termination after the CSP header.
/// @param csp_packet_array_len The length of the `csp_packet_array`.
/// @note This function is only used in this file.
static TCMD_check_result_enum_t check_for_and_handle_new_ax100_kiss_tcmd(
    uint8_t csp_packet_array[], uint16_t csp_packet_array_len
) {
    char latest_tcmd[UART_ax100_buffer_len];
    uint16_t latest_tcmd_len = 0;
    
    // Unnecessary, but good for safety.
    memset(latest_tcmd, 0, UART_ax100_buffer_len);
    latest_tcmd_len = 0; // 0 means no telecommand available (checked later).

    if (csp_packet_array_len == 0) {
        // If we haven't received any telecommands, there is nothing to do here.
        return TCMD_CHECK_STATUS_NO_TCMD;
    }

    // Example of nominal array state:
    // C0 00 C2 18 D1 00 43 54 53 31 2B 68 65 6C 6C 6F 5F 77 6F 72 6C 64 28 29 21 00 27 37 87 EC C0
    // [KISS][---CSP---] C  T  S  1  +  h  e  l  l  o  _  w  o  r  l  d  (  )  !  \0 [---CRC---] [KISS]
    
    // Set `latest_tcmd_len` to the length of the telecommand.
    const int16_t idx_of_last_char_in_tcmd = GEN_get_index_of_substring_in_array(
        (const char*)&csp_packet_array[4], // Skip the first 4 bytes of CSP header
        csp_packet_array_len - 4,
        "!"
    );
    if (idx_of_last_char_in_tcmd < 0) { // -1 indicates "string not found".
        // If the telecommand is not complete, discard it.
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_CRITICAL, LOG_SINK_ALL,
            "Incomplete telecommand received from AX100. Discarding incomplete telecommand."
        );
        return TCMD_CHECK_STATUS_TCMD_INVALID_AND_DISCARDED;
    }
    else {
        // End-of-message character found. Use the buffer up to that point.
        latest_tcmd_len = ((uint16_t) idx_of_last_char_in_tcmd) + 1;
    }
    
    // `memcpy()`, but volatile-compatible.
    for (uint16_t i = 0; i < latest_tcmd_len; i++) {
        latest_tcmd[i] = csp_packet_array[4 + i];
    }
    // Set the null terminator at the end of the `latest_tcmd` str.
    latest_tcmd[latest_tcmd_len] = '\0';

    if (latest_tcmd_len == 0) {
        return TCMD_CHECK_STATUS_NO_TCMD;
    }

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
        return TCMD_CHECK_STATUS_TCMD_INVALID_AND_DISCARDED;
    }

    // Add the telecommand to the agenda (regardless of whether it's in the future).
    TCMD_add_tcmd_to_agenda(&parsed_tcmd);

    return TCMD_CHECK_STATUS_TCMD_SCHEDULED;
}


void TASK_handle_ax100_kiss_telecommands(void *argument) {
    TASK_HELP_start_of_task();

    while (1) {
        // Place the main delay at the top to avoid a "continue" statement skipping it.
        osDelay(400);

        // Continue checking and scheduling telecommands until there are no more to process.
        while (UART_AX100_kiss_frame_queue_tail != UART_AX100_kiss_frame_queue_head) {
            AX100_kiss_frame_struct_t *f = (AX100_kiss_frame_struct_t*)&UART_AX100_kiss_frame_queue[UART_AX100_kiss_frame_queue_tail];
            
            check_for_and_handle_new_ax100_kiss_tcmd(
                f->data, f->len
            );
            // Don't bother checking the result; keep processing all queued frames.

            UART_AX100_kiss_frame_queue_tail = (UART_AX100_kiss_frame_queue_tail + 1) % AX100_MAX_KISS_FRAMES_IN_RX_QUEUE;
        }
    } /* End Task's Main Loop */
}
