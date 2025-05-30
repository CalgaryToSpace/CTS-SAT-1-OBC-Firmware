#include "uart_handler/uart_error_tracking.h"
#include "uart_handler/uart_handler.h"

#include <string.h>
#include <stdio.h>

/* Initiailze UART_Error structs */

UART_error_counts_single_subsystem_struct_t UART_error_mpi_error_info = {
    .parity_error_count = 0,
    .noise_error_count = 0,
    .frame_error_count = 0,
    .overrun_error_count = 0,
    .dma_transfer_error_count = 0,
    .receiver_timeout_error_count = 0,
    .handler_buffer_full_error_count = 0,
};

UART_error_counts_single_subsystem_struct_t UART_error_ax100_error_info = {
    .parity_error_count = 0,
    .noise_error_count = 0,
    .frame_error_count = 0,
    .overrun_error_count = 0,
    .dma_transfer_error_count = 0,
    .receiver_timeout_error_count = 0,
};

UART_error_counts_single_subsystem_struct_t UART_error_gnss_error_info = {
    .parity_error_count = 0,
    .noise_error_count = 0,
    .frame_error_count = 0,
    .overrun_error_count = 0,
    .dma_transfer_error_count = 0,
    .receiver_timeout_error_count = 0,
    .handler_buffer_full_error_count = 0,
};

UART_error_counts_single_subsystem_struct_t UART_error_camera_error_info = {
    .parity_error_count = 0,
    .noise_error_count = 0,
    .frame_error_count = 0,
    .overrun_error_count = 0,
    .dma_transfer_error_count = 0,
    .receiver_timeout_error_count = 0,
    .handler_buffer_full_error_count = 0,
};

UART_error_counts_single_subsystem_struct_t UART_error_eps_error_info = {
    .parity_error_count = 0,
    .noise_error_count = 0,
    .frame_error_count = 0,
    .overrun_error_count = 0,
    .dma_transfer_error_count = 0,
    .receiver_timeout_error_count = 0,
    .handler_buffer_full_error_count = 0,
};

UART_error_counts_single_subsystem_struct_t UART_error_telecommand_error_info = {
    .parity_error_count = 0,
    .noise_error_count = 0,
    .frame_error_count = 0,
    .overrun_error_count = 0,
    .dma_transfer_error_count = 0,
    .receiver_timeout_error_count = 0,
    .handler_buffer_full_error_count = 0,
};

/// @brief Track the error for a given UART instance
/// @param huart_instance The uart instance we are tracking the error for 
/// @param error_code The error code to track
/// @note This function will check if the error code is a known error and increment the corresponding error count
/// @note There can't be any Log_message as this will be called in the HAL_UART_ErrorCallback function 
void UART_track_error_from_isr(USART_TypeDef *huart_instance, uint32_t error_code)
{
    // Assign correct subsystem error info struct

    UART_error_counts_single_subsystem_struct_t *error_info_struct;
    if (huart_instance == NULL) {
        return;
    }

    if (huart_instance == UART_mpi_port_handle->Instance) {
        error_info_struct = &UART_error_mpi_error_info;
    } else if (huart_instance == UART_ax100_port_handle->Instance) {
        error_info_struct = &UART_error_ax100_error_info;
    } else if (huart_instance == UART_gnss_port_handle->Instance) {
        error_info_struct = &UART_error_gnss_error_info;
    } else if (huart_instance == UART_camera_port_handle->Instance) {
        error_info_struct = &UART_error_camera_error_info;
    } else if (huart_instance == UART_eps_port_handle->Instance) {
        error_info_struct = &UART_error_eps_error_info;
    } else if (huart_instance == UART_telecommand_port_handle->Instance) {
        error_info_struct = &UART_error_telecommand_error_info;
    } else {
        return; // Unknown UART instance
    }

    // Check if the error code is a known error
    if (error_code & HAL_UART_ERROR_PE) {
        error_info_struct->parity_error_count++;
    }
    if (error_code & HAL_UART_ERROR_NE) {
        error_info_struct->noise_error_count++;
    }
    if (error_code & HAL_UART_ERROR_FE) {
        error_info_struct->frame_error_count++;
    }
    if (error_code & HAL_UART_ERROR_ORE) {
        error_info_struct->overrun_error_count++;
    }
    if (error_code & HAL_UART_ERROR_DMA) {
        error_info_struct->dma_transfer_error_count++;
    }
    if (error_code & HAL_UART_ERROR_RTO) {
        error_info_struct->receiver_timeout_error_count++;
    }

    // The `handler_buffer_full_error_count` is a custom error that we define
    // to track if the buffer in the ISR handler becomes full.
    // Thus, it will be manully incremented in the HAL_UART_RxCpltCallback function.
}


uint8_t UART_single_subsystem_error_info_to_json(UART_error_counts_single_subsystem_struct_t *error_info_struct, char *json_buffer, uint16_t json_buffer_len)
{
    // Check if the buffer is large enough
    if (json_buffer_len < UART_ERROR_TRACKING_JSON_BUFFER_LEN / 4) {
        return 1; // Buffer is too small
    }
    // Create the JSON string
    snprintf(json_buffer, json_buffer_len,
        "{\"parity\":%u,"
        "\"noise\":%u,"
        "\"frame\":%u,"
        "\"overrun\":%u,"
        "\"dma_transfer\":%u,"
        "\"receiver_timeout\":%u,"
        "\"handler_buffer_full\":%u}",
        error_info_struct->parity_error_count,
        error_info_struct->noise_error_count,
        error_info_struct->frame_error_count,
        error_info_struct->overrun_error_count,
        error_info_struct->dma_transfer_error_count,
        error_info_struct->receiver_timeout_error_count,
        error_info_struct->handler_buffer_full_error_count
    );
    return 0;
}

uint8_t UART_get_errors_json(char *json_buf, uint16_t json_buf_len)
{
    if (json_buf == NULL || json_buf_len < UART_ERROR_TRACKING_JSON_BUFFER_LEN) {
        return 1; // Buffer is NULL or too small
    }

    uint16_t offset = 0;
    int written = snprintf(json_buf + offset, json_buf_len - offset, "{");
    offset += written;

    char buf[UART_ERROR_TRACKING_JSON_BUFFER_LEN / 4];

    // --- Append mpi_errors ---
    memset(buf, 0, sizeof(buf));
    if (UART_single_subsystem_error_info_to_json(&UART_error_mpi_error_info, buf, sizeof(buf))) { return 3;}
    written = snprintf(json_buf + offset, json_buf_len - offset, "\"mpi_errors\":%s", buf);
    if (written < 0 || written >= json_buf_len - offset) { return 4;}
    offset += written;

    // --- Append ax100_errors ---
    memset(buf, 0, sizeof(buf));
    if (UART_single_subsystem_error_info_to_json(&UART_error_ax100_error_info, buf, sizeof(buf))) { return 3;}
    written = snprintf(json_buf + offset, json_buf_len - offset, ",\"ax100_errors\":%s", buf);
    if (written < 0 || written >= json_buf_len - offset) { return 4;}
    offset += written;

    // --- Append gnss_errors ---
    memset(buf, 0, sizeof(buf));
    if (UART_single_subsystem_error_info_to_json(&UART_error_gnss_error_info, buf, sizeof(buf))) { return 3;}
    written = snprintf(json_buf + offset, json_buf_len - offset, ",\"gnss_errors\":%s", buf);
    if (written < 0 || written >= json_buf_len - offset) { return 4;}
    offset += written;

    // --- Append camera_errors ---
    memset(buf, 0, sizeof(buf));
    if (UART_single_subsystem_error_info_to_json(&UART_error_camera_error_info, buf, sizeof(buf))) { return 3;}
    written = snprintf(json_buf + offset, json_buf_len - offset, ",\"camera_errors\":%s", buf);
    if (written < 0 || written >= json_buf_len - offset) { return 4;}
    offset += written;

    // --- Append eps_errors ---
    memset(buf, 0, sizeof(buf));
    if (UART_single_subsystem_error_info_to_json(&UART_error_eps_error_info, buf, sizeof(buf))) { return 3;}
    written = snprintf(json_buf + offset, json_buf_len - offset, ",\"eps_errors\":%s", buf);
    if (written < 0 || written >= json_buf_len - offset) { return 4;}
    offset += written;


    // --- Append telecommand_errors ---
    memset(buf, 0, sizeof(buf));
    if (UART_single_subsystem_error_info_to_json(&UART_error_telecommand_error_info, buf, sizeof(buf))) { return 3;}
    written = snprintf(json_buf + offset, json_buf_len - offset, ",\"telecommand_errors\":%s", buf);
    if (written < 0 || written >= json_buf_len - offset) { return 4;}
    offset += written;

    // --- Close JSON object ---
    if (offset < json_buf_len - 1) {
        json_buf[offset++] = '}';
        json_buf[offset] = '\0';
    } else {
        return 5;
    }

    return 0;
}

