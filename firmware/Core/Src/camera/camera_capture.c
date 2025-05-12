#include "littlefs/littlefs_helper.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_driver.h"
#include "debug_tools/debug_uart.h"
#include "camera/camera_capture.h"
#include "uart_handler/uart_handler.h"
#include "log/log.h"
#include "eps_drivers/eps_channel_control.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "main.h"

/// @brief Timeout duration for camera receive in milliseconds
static const uint32_t CAMERA_RX_TOTAL_TIMEOUT_DURATION_MS = 12000;


static void CAM_end_camera_receive_due_to_error(lfs_file_t* img_file) {
    // Close file if open.
    if (img_file != NULL) {
        const int8_t close_result = lfs_file_close(&LFS_filesystem, img_file);
        if (close_result != 0) {
            LOG_message(
                LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING,
                LOG_all_sinks_except(LOG_SINK_FILE), "Error closing file (err %d)", close_result
            );
        }
    }

    // Turn off camera before exiting.
    const uint8_t eps_off_status = EPS_set_channel_enabled(EPS_CHANNEL_3V3_CAMERA, 0);
    if (eps_off_status != 0)
    {
        // Continue anyway. Just log a warning.
        LOG_message(
            LOG_SYSTEM_EPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Error disabling camera power channel in CAM_capture_image: status=%d. Continuing.",
            eps_off_status
        );
    }
}


/// @brief Capture an image, writing to a file in LFS.
/// @return 0: Success, 3: Failed UART reception, 4: Timeout while waiting for data
static uint8_t CAM_receive_image(lfs_file_t* img_file) {
    CAMERA_uart_half_1_state = CAMERA_UART_WRITE_STATE_IDLE;
    CAMERA_uart_half_2_state = CAMERA_UART_WRITE_STATE_IDLE;

    // Set start time and start receiving.
    const uint32_t UART_camera_rx_start_time_ms = HAL_GetTick();
    const uint8_t receive_status = CAMERA_set_expecting_data(1);
    // Check for UART reception errors
    if (receive_status == 3) {
        CAMERA_set_expecting_data(0);
        return 3; // Error code: Failed UART reception
    }

    uint32_t total_bytes_written = 0;
    uint8_t total_buffers_filled = 0;

    while(1) {
        // Write file after half callback (Half 1).
        if (CAMERA_uart_half_1_state == CAMERA_UART_WRITE_STATE_HALF_FILLED_WAITING_FS_WRITE) {
            total_buffers_filled++;

            DEBUG_uart_print_str("From while loop, half 1 filled: ");
            DEBUG_uart_print_str_max_len(
                (const char*)UART_camera_pending_fs_write_half_1_buf, 11
            );
            DEBUG_uart_print_str("\n");

            DEBUG_uart_print_str("Last sentence half 1: ");
            const uint16_t last_sentence_start = CAM_SENTENCE_LEN * (CAM_SENTENCES_PER_HALF_CALLBACK - 1); 
            DEBUG_uart_print_str_max_len(
                (const char*)UART_camera_pending_fs_write_half_1_buf + last_sentence_start, CAM_SENTENCE_LEN
            );
            DEBUG_uart_print_str("\n");
            
            // Write data to file
            const lfs_ssize_t write_result = lfs_file_write(
                &LFS_filesystem, img_file,
                (const uint8_t *)UART_camera_pending_fs_write_half_1_buf,
                UART_camera_dma_buffer_len_half
            );
            if (write_result < 0) {
                LOG_message(
                    LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
                    "LFS error writing half 1 to img file: %ld.",
                    write_result
                );
            } else {
                total_bytes_written += write_result;
            }

            CAMERA_uart_half_1_state = CAMERA_UART_WRITE_STATE_HALF_WRITTEN_TO_FS;
        }

        // Write file after complete callback (Half 2).
        if (CAMERA_uart_half_2_state == CAMERA_UART_WRITE_STATE_HALF_FILLED_WAITING_FS_WRITE) {
            total_buffers_filled++;

            DEBUG_uart_print_str("From while loop, half 2 filled: ");
            DEBUG_uart_print_str_max_len(
                (const char*)UART_camera_pending_fs_write_half_2_buf, 11
            );
            DEBUG_uart_print_str("\n");

            DEBUG_uart_print_str("Last sentence half 2: ");
            const uint16_t last_sentence_start = CAM_SENTENCE_LEN * (CAM_SENTENCES_PER_HALF_CALLBACK - 1); 
            DEBUG_uart_print_str_max_len(
                (const char*)UART_camera_pending_fs_write_half_2_buf + last_sentence_start, CAM_SENTENCE_LEN
            );
            DEBUG_uart_print_str("\n");

            // Write data to file
            const lfs_ssize_t write_result = lfs_file_write(
                &LFS_filesystem, img_file,
                (const uint8_t *)UART_camera_pending_fs_write_half_2_buf, UART_camera_dma_buffer_len_half
            );
            if (write_result < 0) {
                LOG_message(
                    LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
                    "LFS error writing half 2 to img file: %ld.",
                    write_result
                );
            }
            else {
                total_bytes_written += write_result;
            }

            CAMERA_uart_half_2_state = CAMERA_UART_WRITE_STATE_HALF_WRITTEN_TO_FS;
        }

        // Timeout condition: If the total time has exceeded CAMERA_RX_TOTAL_TIMEOUT_DURATION_MS.
        if ((HAL_GetTick() - UART_camera_rx_start_time_ms) > CAMERA_RX_TOTAL_TIMEOUT_DURATION_MS) {
            LOG_message(
                LOG_SYSTEM_BOOM, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
                "Camera receiving exceeded CAMERA_RX_TOTAL_TIMEOUT_DURATION_MS duration (%ldms). Breaking out of loop.",
                CAMERA_RX_TOTAL_TIMEOUT_DURATION_MS
            );

            // If both states are still idle (ie, no data received), return error 4.
            if ((CAMERA_uart_half_1_state == CAMERA_UART_WRITE_STATE_IDLE) &&
                (CAMERA_uart_half_2_state == CAMERA_UART_WRITE_STATE_IDLE)) {
                CAMERA_set_expecting_data(0);
                return 4; // Error code: Timeout waiting for first byte
            }

            // Otherwise, break out of the loop and write any remaining data.
            break;
        }

        // Between messages timeout condition: 
        // If it has been more than 6 seconds since starting to capture image AND
        // if it has been more than 2 seconds since the last write time
        // (indicating that the camera is not sending data).
        // Break out of the loop.
        // This is the nominal exit condition.
        const uint32_t current_time = HAL_GetTick();
        if (((current_time - UART_camera_rx_start_time_ms) > 6000) && // Allow 6000ms for the first message
            ((current_time - UART_camera_last_write_time_ms) > 2000)
        ) {
            LOG_message(
                LOG_SYSTEM_BOOM, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
                "Camera hasn't written data in 2 seconds (assuming done). Breaking out of loop."
            );
            break;
        }
    }

    LOG_message(
        LOG_SYSTEM_BOOM, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE),
        "Camera loop finished. total_bytes_written=%ld. total_buffers_filled=%d",
        total_bytes_written,
        total_buffers_filled
    );

    CAMERA_set_expecting_data(0);

    if (CAMERA_uart_half_1_state == CAMERA_UART_WRITE_STATE_HALF_FILLING
        && CAMERA_uart_half_2_state == CAMERA_UART_WRITE_STATE_HALF_FILLING
    ) {
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "Weird state where both halfs say they're filling (after the rx loop)."
        );
    }
    
    // Read any remaining data in the DMA buffer.
    volatile uint8_t *remaining_data_array = NULL; // Pointer to a volatile array.
    if (CAMERA_uart_half_1_state == CAMERA_UART_WRITE_STATE_HALF_FILLING) {
        remaining_data_array = (uint8_t*) &UART_camera_dma_buffer[0];
    } else if (CAMERA_uart_half_2_state == CAMERA_UART_WRITE_STATE_HALF_FILLING) {
        remaining_data_array = (uint8_t*) &UART_camera_dma_buffer[UART_camera_dma_buffer_len_half];
    }
    else {
        remaining_data_array = NULL;

        // No remaining data to write
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
            "No remaining data to write."
        );
    }

    if (remaining_data_array != NULL) {
        // Print remaining data
        DEBUG_uart_print_str("Remaining data in DMA buffer: ");
        DEBUG_uart_print_str_max_len(
            (const char *)remaining_data_array, CAM_SENTENCE_LEN);
        DEBUG_uart_print_str("\n");

        const uint32_t remaining_data_len = strnlen(
            (const char *)remaining_data_array, UART_camera_dma_buffer_len_half
        );
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_FILE),
            "Remaining data length: %ld", remaining_data_len
        );

        // Write remaining data to file
        const lfs_ssize_t write_result = lfs_file_write(
            &LFS_filesystem, img_file,
            (const uint8_t *)remaining_data_array,
            remaining_data_len
        );
        if (write_result < 0) {
            LOG_message(
                LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
                "LFS error writing remaining data to img file: %ld.",
                write_result
            );
        } else {
            total_bytes_written += write_result;
        }
    }
    
    LOG_message(
        LOG_SYSTEM_BOOM, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE),
        "Total camera write to file: %ld bytes (%.2f = 0x%04lX sentences). total_buffers_filled=%d.",
        total_bytes_written,
        ((float)total_bytes_written) / ((float)CAM_SENTENCE_LEN),
        total_bytes_written / CAM_SENTENCE_LEN,
        total_buffers_filled
    );

    return 0;
}



/// @brief Capture an image, writing to a file in LFS.
/// @param filename_str The name of the file to write to.
/// @param lighting_mode Should be a lower-case char
///         d - daylight ambient light
///         m - medium ambient light
///         n - night ambient light
///         s - solar sail contrast and light
/// @return 0: Successfully captured image, more than 0, error occurred
/// @note This function does not validate that the camera is connected or powered on.
CAM_capture_status_enum CAM_capture_image(char filename_str[], char lighting_mode) {
    if (!(lighting_mode == 'd' || lighting_mode == 'm' || lighting_mode == 'n' || lighting_mode == 's')) {
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error: Invalid lighting mode: %c", lighting_mode
        );
        return CAM_CAPTURE_STATUS_WRONG_INPUT;
    }

    // If lfs not mounted, mount.
    if (!LFS_is_lfs_mounted) {
        if (LFS_mount() != 0) {
            LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Error mounting LFS filesystem");
            return CAM_CAPTURE_STATUS_LFS_NOT_MOUNTED;
        }
    }

    // Create and open file before receive loop.
    
    // Open LFS file for writing.
    lfs_file_t img_file;
    const int8_t open_result = lfs_file_opencfg(
        &LFS_filesystem, &img_file,
        filename_str,
        LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC,
        &LFS_file_cfg
    );
    if (open_result < 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "Error opening / creating file: %s", filename_str
        );
        CAM_end_camera_receive_due_to_error(&img_file);
        return CAM_CAPTURE_STATUS_LFS_FAILED_OPENING_CREATING_FILE;
    }

    // Write a tiny header to file, as the first write takes longer than subsequent writes.
    const char * const header_str = "START_CAM:\n";
    const lfs_ssize_t write_result = lfs_file_write(
        &LFS_filesystem, &img_file,
        (const uint8_t *)header_str, strlen(header_str)
    );
    if (write_result < 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "LFS error writing header to img file."
        );
        CAM_end_camera_receive_due_to_error(&img_file);
        return CAM_CAPTURE_STATUS_LFS_FAILED_WRITING_HEADER;
    }
    
    LOG_message(
        LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE),
        "Opened image file: %s",
        filename_str
    );
    
    // Clear camera response buffer (volatile-safe memset).
    for (uint16_t i = 0; i < UART_camera_dma_buffer_len; i++) {
        UART_camera_dma_buffer[i] = 0;
    }
    // Clear rx buf (probably not essential).
    for (uint16_t i = 0; i < UART_camera_dma_buffer_len_half; i++) {
        UART_camera_pending_fs_write_half_1_buf[i] = 0;
        UART_camera_pending_fs_write_half_2_buf[i] = 0;
    }
    CAMERA_uart_half_1_state = CAMERA_UART_WRITE_STATE_IDLE;
    CAMERA_uart_half_2_state = CAMERA_UART_WRITE_STATE_IDLE;

    // Trigger the camera to start capturing.
    const HAL_StatusTypeDef tx_status = HAL_UART_Transmit(
        UART_camera_port_handle, (uint8_t*)&lighting_mode, 1, 1000
    );
        
    if (tx_status != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error sending camera command: HAL_UART_Transmit status=%d",
            tx_status);
        // Close file.
        CAM_end_camera_receive_due_to_error(&img_file);
        return CAM_CAPTURE_STATUS_FAILED_TRANSMITTING_LIGHTING_MODE;
    }

    const uint8_t capture_code = CAM_receive_image(&img_file);

    // Turn off the camera.
    const uint8_t eps_off_status = EPS_set_channel_enabled(EPS_CHANNEL_3V3_CAMERA, 0);
    if (eps_off_status != 0) {
        // Continue anyway. Just log a warning.
        LOG_message(
            LOG_SYSTEM_EPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Error disabling camera power channel in CAM_capture_image: status=%d. Continuing.",
            eps_off_status
        );
    }

    // Close file.
    const int8_t close_result = lfs_file_close(&LFS_filesystem, &img_file);
    if (close_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "Error closing file: %s", filename_str
        );
        return CAM_CAPTURE_STATUS_LFS_FAILED_CLOSING_FILE;
    }

    if (capture_code != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error receiving camera image: Capture Code = %d",
            capture_code
        );
        return CAM_CAPTURE_STATUS_CAPTURE_FAILURE;
    }
    return CAM_CAPTURE_STATUS_TRANSMIT_SUCCESS;
}

void CAM_repeated_error_log_message() {
    LOG_message(
        LOG_SYSTEM_BOOM, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
        "If this repeatedly fails, do the following:\n"
        "1. Turn off the EPS channel for the camera.\n"
        "2. Wait a minute.\n"
        "3. Manually change the baudrate of the camera to 115200.\n"
        "4. Start the process again from camera_setup."
    );
}
