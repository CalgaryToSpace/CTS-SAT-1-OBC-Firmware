#include "uart_handler/uart_error_tracking.h"
#include "uart_handler/uart_handler.h"

#include <string.h>
#include <stdio.h>

/* Initiailze UART_Error structs */

UART_Error_Info_t UART_Error_mpi_error_info = {
    .parity_error_count = 0,
    .noise_error_count = 0,
    .frame_error_count = 0,
    .overrun_error_count = 0,
    .dma_transfer_error_count = 0,
    .receiver_timeout_error_count = 0,
};

UART_Error_Info_t UART_Error_gps_error_info = {
    .parity_error_count = 0,
    .noise_error_count = 0,
    .frame_error_count = 0,
    .overrun_error_count = 0,
    .dma_transfer_error_count = 0,
    .receiver_timeout_error_count = 0,
};

UART_Error_Info_t UART_Error_camera_error_info = {
    .parity_error_count = 0,
    .noise_error_count = 0,
    .frame_error_count = 0,
    .overrun_error_count = 0,
    .dma_transfer_error_count = 0,
    .receiver_timeout_error_count = 0,

};

UART_Error_Info_t UART_Error_eps_error_info = {
    .parity_error_count = 0,
    .noise_error_count = 0,
    .frame_error_count = 0,
    .overrun_error_count = 0,
    .dma_transfer_error_count = 0,
    .receiver_timeout_error_count = 0,
};

/// @brief Track the error for a given UART instance
/// @param huart_instance The uart instance we are tracking the error for 
/// @param error_code The error code to track
/// @note This function will check if the error code is a known error and increment the corresponding error count
/// @note There can't be any Log_message as this will be called in the HAL_UART_ErrorCallback function 
void UART_Error_tracking(USART_TypeDef *huart_instance, uint32_t error_code)
{
    UART_Error_Info_t error_info_struct;

    if (UART_Error_tracking_get_tracking_struct_from_uart_instance(huart_instance, &error_info_struct) != 0) {
        return;
    }
    // Check if the error code is a known error
    if (error_code & HAL_UART_ERROR_PE) {
        error_info_struct.parity_error_count++;
    }
    if (error_code & HAL_UART_ERROR_NE) {
        error_info_struct.noise_error_count++;
    }
    if (error_code & HAL_UART_ERROR_FE) {
        error_info_struct.frame_error_count++;
    }
    if (error_code & HAL_UART_ERROR_ORE) {
        error_info_struct.overrun_error_count++;
    }
    if (error_code & HAL_UART_ERROR_DMA) {
        error_info_struct.dma_transfer_error_count++;
    }
    if (error_code & HAL_UART_ERROR_RTO) {
        error_info_struct.receiver_timeout_error_count++;
    }
}

/// @brief Get the error tracking struct for a given UART instance
/// @param huart_instance The uart instance we are tracking the error for 
/// @param result_error_info_struct Pointer to the error info struct to be filled 
/// @return 0 if the struct was found, 1 if not 
uint8_t UART_Error_tracking_get_tracking_struct_from_uart_instance(USART_TypeDef *huart_instance, UART_Error_Info_t *result_error_info_struct) 
{
    if (huart_instance == UART_mpi_port_handle->Instance) {
        result_error_info_struct = &UART_Error_mpi_error_info;
        return 0;
    }

    if (huart_instance == UART_gps_port_handle->Instance) {
        result_error_info_struct = &UART_Error_gps_error_info;
        return 0;
    }

    if (huart_instance == UART_camera_port_handle->Instance) {
        result_error_info_struct = &UART_Error_camera_error_info;
        return 0;
    }

    if (huart_instance == UART_eps_port_handle->Instance) {
        result_error_info_struct = &UART_Error_eps_error_info;
        return 0;
    }
    // Initialize the error info struct to 0 to appease warnings
    // not going to use anyways
    result_error_info_struct->dma_transfer_error_count = 0;
    result_error_info_struct->frame_error_count = 0;
    result_error_info_struct->noise_error_count = 0;
    result_error_info_struct->overrun_error_count = 0;
    result_error_info_struct->parity_error_count = 0;
    result_error_info_struct->receiver_timeout_error_count = 0; 
    return 1;
}

uint8_t UART_Error_tracking_subsystem_error_info_to_json(UART_Error_Info_t *error_info_struct, char *json_buffer, uint16_t json_buffer_len)
{
    // Check if the buffer is large enough
    if (json_buffer_len < UART_ERROR_TRACKING_JSON_BUFFER_LEN / 4) {
        return 1; // Buffer is too small
    }
    // Create the JSON string
    snprintf(json_buffer, json_buffer_len,
        "{\"parity_error_count\":%u,\"noise_error_count\":%u,\"frame_error_count\":%u,\"overrun_error_count\":%u,\"dma_transfer_error_count\":%u,\"receiver_timeout_error_count\":%u}",
        error_info_struct->parity_error_count,
        error_info_struct->noise_error_count,
        error_info_struct->frame_error_count,
        error_info_struct->overrun_error_count,
        error_info_struct->dma_transfer_error_count,
        error_info_struct->receiver_timeout_error_count
    );
    return 0;
}

uint8_t UART_Error_tracking_get_errors_json(char *json_buf, uint16_t json_buf_len)
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
    if (UART_Error_tracking_subsystem_error_info_to_json(&UART_Error_mpi_error_info, buf, sizeof(buf))) return 3;
    written = snprintf(json_buf + offset, json_buf_len - offset, "\"mpi_errors\":%s", buf);
    if (written < 0 || written >= json_buf_len - offset) return 4;
    offset += written;

    // --- Append gps_errors ---
    memset(buf, 0, sizeof(buf));
    if (UART_Error_tracking_subsystem_error_info_to_json(&UART_Error_gps_error_info, buf, sizeof(buf))) return 3;
    written = snprintf(json_buf + offset, json_buf_len - offset, ",\"gps_errors\":%s", buf);
    if (written < 0 || written >= json_buf_len - offset) return 4;
    offset += written;

    // --- Append camera_errors ---
    memset(buf, 0, sizeof(buf));
    if (UART_Error_tracking_subsystem_error_info_to_json(&UART_Error_camera_error_info, buf, sizeof(buf))) return 3;
    written = snprintf(json_buf + offset, json_buf_len - offset, ",\"camera_errors\":%s", buf);
    if (written < 0 || written >= json_buf_len - offset) return 4;
    offset += written;

    // --- Append eps_errors ---
    memset(buf, 0, sizeof(buf));
    if (UART_Error_tracking_subsystem_error_info_to_json(&UART_Error_eps_error_info, buf, sizeof(buf))) return 3;
    written = snprintf(json_buf + offset, json_buf_len - offset, ",\"eps_errors\":%s", buf);
    if (written < 0 || written >= json_buf_len - offset) return 4;
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

