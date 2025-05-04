#include "littlefs/littlefs_helper.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_driver.h"
#include "debug_tools/debug_uart.h"
#include "camera/camera_init.h"
#include "uart_handler/uart_handler.h"
#include "log/log.h"
#include "eps_drivers/eps_channel_control.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "main.h"
#include "camera_init.h"

/// @brief Timeout duration for camera receive in milliseconds
static const uint32_t CAMERA_RX_TOTAL_TIMEOUT_DURATION_MS = 8000;


/// @brief Changes the baudrate of the camera by sending it a UART command, and then changing the
///     baudrate of the camera UART port.
/// @param new_baud_rate The new baud rate to set the camera to. Ranges from 1200 to 921600.
/// @return 0 if successful, 1 if an invalid baud rate was provided, >=2 if an error occurred
/// @note This function does not validate that the camera is connected or powered on.
uint8_t CAM_change_baudrate(uint32_t new_baud_rate) {
    char* set_baudrate_tx_code;

    if (new_baud_rate == 1200) {
        set_baudrate_tx_code = "0";
    } else if (new_baud_rate == 2400) {
        set_baudrate_tx_code = "1";
    } else if (new_baud_rate == 9600) {
        set_baudrate_tx_code = "2";
    } else if (new_baud_rate == 19200) {
        set_baudrate_tx_code = "3";
    } else if (new_baud_rate == 38400) {
        set_baudrate_tx_code = "4";
    } else if (new_baud_rate == 57600) {
        set_baudrate_tx_code = "5";
    } else if (new_baud_rate == 115200) {
        set_baudrate_tx_code = "6";
    } else if (new_baud_rate == 230400) {
        set_baudrate_tx_code = "7";
    } else if (new_baud_rate == 460800) {
        set_baudrate_tx_code = "8";
    } else if (new_baud_rate == 921600) {
        set_baudrate_tx_code = "9";
    } else {
        return 1; // Invalid baud rate
    }

    const HAL_StatusTypeDef tx_status = HAL_UART_Transmit(
        UART_camera_port_handle, (uint8_t*)set_baudrate_tx_code, 1, 1000
    );
    if (tx_status != HAL_OK) {
        return 2; // Error
    }

    // Wait for UART transmission to complete.
    HAL_Delay(50); // 25 is probably enough.

    // Deinit camera uart port and reinitialize with new baudrate.
    const HAL_StatusTypeDef de_init_status = HAL_UART_DeInit(UART_camera_port_handle);
    if (de_init_status != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Error de-init camera UART port in CAM_change_baudrate: status=%d. Continuing.",
            de_init_status
        );
        // Note: Do not return here. Continue on, even if it fails.
    }

    UART_camera_port_handle->Init.BaudRate = new_baud_rate;
    const HAL_StatusTypeDef init_status = HAL_UART_Init(UART_camera_port_handle);
    if (init_status != HAL_OK) {
        return 3;
    }

    return 0; // Success
}


/// @brief Set up the camera by powering on and changing the baudrate to 2400.
/// @return 0 on success. The error code from the `CAM_change_baudrate` function, or >100 if an EPS error occurred.
/// @note Does not perform a self-test.
uint8_t CAM_setup() {
    // First, turn on the camera.
    const uint8_t eps_status = EPS_set_channel_enabled(EPS_CHANNEL_3V3_CAMERA, 1);
    if (eps_status != 0) {
        // Continue anyway. Just log a warning.
        LOG_message(
            LOG_SYSTEM_EPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Error enabling camera power channel in CAM_setup: status=%d. Continuing.",
            eps_status
        );
    }

    // Wait a sec for camera bootup.
    HAL_Delay(500);

    // Viable baud rate options: 115200, 230400.
    const uint8_t bitrate_status = CAM_change_baudrate(230400);
    if (bitrate_status != 0) {
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error changing camera baudrate: CAM_change_baudrate returned %d",
            bitrate_status
        );

        // Turn off camera before exiting
        const uint8_t eps_off_status = EPS_set_channel_enabled(EPS_CHANNEL_3V3_CAMERA, 0);
        if (eps_off_status != 0) {
            // Continue anyway. Just log a warning.
            LOG_message(
                LOG_SYSTEM_EPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                "Error disabling camera power channel in CAM_capture_image: status=%d. Continuing.",
                eps_off_status
            );
        }
        return 2;
    }
    return 0; // Success
}

/// @brief Send the 't' test command to the camera, and check if the response is valid.
/// @return 0 if successful, >0 if an error occurred.
/// @note You should call `CAM_setup` before calling this function.
uint8_t CAM_test() {
    // We're about to use blocking mode. Abort any waiting DMA registrations.
    HAL_UART_AbortReceive(UART_camera_port_handle);

    // Send the test command to the camera.
    const HAL_StatusTypeDef tx_status =  HAL_UART_Transmit(
        UART_camera_port_handle, (uint8_t*)"t", 1, 1000
    );
    if (tx_status != HAL_OK) {
        return 1; // Error
    }

    // Do a blocking receive of the response.
    // Seems that the test sentence response could be 201 bytes. 202 fails sometimes.
    // Receive slightly less for safety (200).
    char UART_test_str[200];

    // Preset in order to check if the response is empty.
    memset(UART_test_str, 0, sizeof(UART_test_str));

    const HAL_StatusTypeDef rx_status = HAL_UART_Receive(
        UART_camera_port_handle, (uint8_t*) UART_test_str, sizeof(UART_test_str),
        // Timeout: At 1200 baud, the command takes <=1882ms. Using 2000ms here ensures it works
        // for all baud rates (including the minimum).
        2000
    );
    if (rx_status != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error receiving camera test response: HAL_UART_Receive status=%d",
            rx_status
        );

        return 3; // Error
    }

    // Safety: Ensure the string is null-terminated.
    UART_test_str[sizeof(UART_test_str) - 1] = 0;

    LOG_message(
        LOG_SYSTEM_BOOM, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "Camera test response:\n%s\nEND RESPONSE\n",
        UART_test_str
    );

    if (UART_test_str[0] == 0) {
        return 5;
    }

    // Should contain string "piCAM/FM Test String Transmission".
    if (strstr(UART_test_str, "piCAM/FM Test String Transmission") == NULL) {
        return 6;
    }

    const char* const test_resp_border_str = "@----------------------------------------------------------------";

    // Ensure string starts with the `test_resp_border_str`.
    if (strncmp(UART_test_str, test_resp_border_str, strlen(test_resp_border_str)) != 0) {
        return 7;
    }

    // Ensure there's a second border string in the response.
    if (strstr(UART_test_str + strlen(test_resp_border_str), test_resp_border_str) == NULL) {
        return 8;
    }

    return 0;
}

/// @brief Capture an image, writing to a file in LFS.
/// @return 0: Success, 3: Failed UART reception, 4: Timeout while waiting for data
uint8_t CAM_receive_image(lfs_file_t* img_file) {
    // Set start time and start receiving.
    const uint32_t UART_camera_rx_start_time_ms = HAL_GetTick();
    const uint8_t receive_status = CAMERA_set_expecting_data(1);
    // Check for UART reception errors
    if (receive_status == 3) {
        CAMERA_set_expecting_data(0);
        return 3; // Error code: Failed UART reception
    }

    uint32_t total_bytes_written = 0;

    while(1) {
        // Write file after half callback (Half 1).
        if (CAMERA_uart_half_1_state == CAMERA_UART_WRITE_STATE_HALF_FILLED_WAITING_FS_WRITE) {
            DEBUG_uart_print_str("From while loop, half 1 filled: ");
            DEBUG_uart_print_str_max_len(
                (const char*)UART_camera_pending_fs_write_half_1_buf, 11
            );
            DEBUG_uart_print_str("\n");

            // Write data to file
            const lfs_ssize_t write_result = lfs_file_write(
                &LFS_filesystem, img_file,
                (const uint8_t *)UART_camera_pending_fs_write_half_1_buf,
                CAM_SENTENCE_LEN*CAM_SENTENCES_PER_HALF_CALLBACK
            );
            if (write_result < 0) {
                LOG_message(
                    LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
                    "LFS error writing half 1 to img file."
                );
            }
            else {
                total_bytes_written += write_result;
            }

            CAMERA_uart_half_1_state = CAMERA_UART_WRITE_STATE_HALF_WRITTEN_TO_FS;
            // return 0; // FIXME: THIS RETURN SHOULD BE REMOVED. For debug only. // FIXME: Start here, consider issue of probably need to delete the file better maybe.
        }

        // Write file after complete callback (Half 2).
        if (CAMERA_uart_half_2_state == CAMERA_UART_WRITE_STATE_HALF_FILLED_WAITING_FS_WRITE) {
            DEBUG_uart_print_str("From while loop, half 2 filled: ");
            DEBUG_uart_print_str_max_len(
                (const char*)UART_camera_pending_fs_write_half_2_buf, 11
            );
            DEBUG_uart_print_str("\n");

            // Write data to file
            const lfs_ssize_t write_result = lfs_file_write(
                &LFS_filesystem, img_file,
                (const uint8_t *)UART_camera_pending_fs_write_half_2_buf, CAM_SENTENCE_LEN*CAM_SENTENCES_PER_HALF_CALLBACK
            );
            if (write_result < 0) {
                LOG_message(
                    LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
                    "LFS error writing half 2 to img file."
                );
            }
            else {
                total_bytes_written += write_result;
            }

            CAMERA_uart_half_2_state = CAMERA_UART_WRITE_STATE_HALF_WRITTEN_TO_FS;
        }

        // Timeout condition: If the total time has exceeded 10 seconds.
        if ((HAL_GetTick() - UART_camera_rx_start_time_ms) > CAMERA_RX_TOTAL_TIMEOUT_DURATION_MS) {
            // If both states are still idle (ie, no data received), return error 4.
            if (CAMERA_uart_half_1_state == CAMERA_UART_WRITE_STATE_IDLE &&
                CAMERA_uart_half_2_state == CAMERA_UART_WRITE_STATE_IDLE) {
                CAMERA_set_expecting_data(0);
                return 4; // Error code: Timeout waiting for first byte
            }

            // Otherwise, break out of the loop and write any remaining data.
            break;
        }

        // FIXME: Should add a between-messages timeout here, which looks at UART_camera_last_write_time_ms
    }

    LOG_message(
        LOG_SYSTEM_BOOM, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_FILE),
        "Camera receive loop finished. Total bytes written: %ld", total_bytes_written
    );

    // Try to read any remaining data in the DMA buffer.
    HAL_UART_DMAStop(UART_camera_port_handle);
    if (CAMERA_uart_half_1_state == CAMERA_UART_WRITE_STATE_HALF_FILLING
        && CAMERA_uart_half_2_state == CAMERA_UART_WRITE_STATE_HALF_FILLING
    ) {
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "Weird state where both halfs say they're filling (after the rx loop)."
        );
    }

    // FIXME: Read out the rest of the data in the buffers here.

    LOG_message(
        LOG_SYSTEM_BOOM, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_FILE),
        "Total bytes written to file: %ld (approx %ld = 0x%04lX sentences)",
        total_bytes_written,
        total_bytes_written / CAM_SENTENCE_LEN,
        total_bytes_written / CAM_SENTENCE_LEN
    );

    // FIXME: Need to handle reading the final half of the data (implemented somewhat in the following commented block)
    
    return 0;
}


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
/// @param filename_str The name of the file to write to.
/// @param lighting_mode Should be a lower-case char
///         d - daylight ambient light
///         m - medium ambient light
///         n - night ambient light
///         s - solar sail contrast and light
/// @return 0: Successfully captured image, Wrong_input: invalid parameter input, Capture_Failure: Error in image reception or command transmission
enum CAM_capture_status_enum CAM_capture_image(char filename_str[], char lighting_mode) {
    if (!(lighting_mode == 'd' || lighting_mode == 'm' || lighting_mode == 'n' || lighting_mode == 's')) {
        return CAM_CAPTURE_STATUS_WRONG_INPUT;
    }

    // If lfs not mounted, mount.
    if (!LFS_is_lfs_mounted) {
        if (LFS_mount() != 0) {
            LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Error mounting LFS filesystem");
            return 1;
        }
    }

    // Create and open file before receive loop.
    LFS_delete_file(filename_str);
    
    // Open LFS file for writing.
    lfs_file_t img_file;
    const int8_t open_result = lfs_file_opencfg(
        &LFS_filesystem, &img_file,
        filename_str,
        LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND,
        &LFS_file_cfg
    );
    if (open_result < 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "Error opening / creating file: %s", filename_str
        );
        CAM_end_camera_receive_due_to_error(&img_file);
        return 2;
    }

    // Write a tiny header to file, as the first write takes longer than subsequent writes.
    const char * const header_str = "START_CAM:";
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
        return 3;
    }
    
    LOG_message(
        LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE),
        "Opened image file: %s",
        filename_str
    );
    
    // Clear camera response buffer (volatile-safe memset).
    for (uint16_t i = 0; i < UART_camera_buffer_len; i++) {
        UART_camera_dma_buffer[i] = 0;
    }
    // Clear rx buf (probably not essential).
    for (uint16_t i = 0; i < CAM_SENTENCE_LEN*CAM_SENTENCES_PER_HALF_CALLBACK; i++){
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
        return CAM_CAPTURE_STATUS_CAPTURE_FAILURE;
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
        return CAM_CAPTURE_STATUS_CAPTURE_FAILURE;
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
