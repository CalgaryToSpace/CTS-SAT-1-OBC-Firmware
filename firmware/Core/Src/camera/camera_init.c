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

/// @brief Timeout duration for camera receive in milliseconds
static const uint32_t CAMERA_RX_TIMEOUT_DURATION_MS =12000;

/// @brief Global variables for file and file_open 
uint8_t file_open = 0;
lfs_file_t file;
const char file_name[] = "image1_0";

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
            LOG_SYSTEM_BOOM, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Error enabling camera power channel in CAM_setup: status=%d. Continuing.",
            eps_status
        );
    }

    // Wait a sec for camera bootup.
    HAL_Delay(500);

    // Change baudrate to 230400 on camera.
    const uint8_t bitrate_status = CAM_change_baudrate(230400);
    if (bitrate_status != 0) {
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error changing camera baudrate: CAM_change_baudrate returned %d",
            bitrate_status
        );
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

/// @brief deals with receiving image in chunks and writing to LFS
/// @return 0: Success, 3: Failed UART reception, 4: Timeout while waiting for data
uint8_t CAM_receive_image(){
    // Clear camera response buffer (Note: Can't use memset because UART_camera_buffer is Volatile)
    for (uint16_t i = 0; i < UART_camera_buffer_len; i++) {
        UART_camera_buffer[i] = 0;
    }
    // clear rx buf
    for (uint16_t i = 0; i < CAM_SENTENCE_LEN*23; i++){
        UART_camera_rx_buf[i] = 0;
    }
    // reset variables
    camera_write_file = 0;
    UART_camera_buffer_write_idx = 0;
    UART_camera_last_write_time_ms = 0;

    // set start time and start receiving
    const uint32_t UART_camera_rx_start_time_ms = HAL_GetTick();
    const uint8_t receive_status = CAMERA_set_expecting_data(1);
    // Check for UART reception errors
    if (receive_status == 3) {
        CAMERA_set_expecting_data(0);
        return 3; // Error code: Failed UART reception
    }

    

    while(1){
        // receive until response timed out

        // write file after half and complete callbacks
        if (camera_write_file){
            // debug string DELETE THIS AFTER TESTING
            DEBUG_uart_print_str("in write file\n");
            UART_camera_last_write_time_ms = HAL_GetTick();

            // Write data to file
            const lfs_ssize_t write_result = lfs_file_write(&LFS_filesystem, &file, UART_camera_rx_buf, CAM_SENTENCE_LEN*23);
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
                    CAMERA_set_expecting_data(0);
                    return 4; // Error code: Timeout waiting for first byte
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
                    break;
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
                    break;
                }
        }
    }
    // outside of while loop
    // write remaining data if any

    if (camera_write_file){
        // debug string DELETE THIS AFTER TESTING
        DEBUG_uart_print_str("in write file - outside loop\n");
        UART_camera_last_write_time_ms = HAL_GetTick();

        // Write data to file
        const lfs_ssize_t write_result = lfs_file_write(&LFS_filesystem, &file, UART_camera_rx_buf, CAM_SENTENCE_LEN*23);
        if (write_result < 0)
        {
            LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Error writing to file: %s", file_name);
        }

        camera_write_file = 0;
    }

    // close file before leaving function if file open
    if (file_open) {
        const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
        if (close_result < 0)
        {
            LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Error closing file: %s", file_name);
        } else {
            LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "File: %s succesfully closed", file_name);
        }
        file_open = 0;
    }

    return 0;
}


/// @brief Transmits ASCII telecommand to take image with different lighting modes
/// @param lighting Should be a lower-case char
///         d - daylight ambient light
///         m - medium ambient light
///         n - night ambient light
///         s - solar sail contrast and light
/// @return Transmit_Success: Successfully captured image, Wrong_input: invalid parameter input, Capture_Failure: Error in image reception or command transmition
enum CAM_capture_status_enum CAM_Capture_Image(bool enable_flash, char lighting_mode){
    // ignore flash for now, I'll hardcode it to False since we don't have flash anyways
    // default file open to false
    file_open = 0;
    // if lfs not mounted, mount
    if (!LFS_is_lfs_mounted) {
        LFS_mount();
    }

    // create and open file before receive loop
    // file name hardcoded for now
    // TODO turn file name into parameter
    LFS_delete_file(file_name);
    if (file_open == 0){
        const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND, &LFS_file_cfg);
        if (open_result < 0) {
            LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Error opening / creating file: %s", file_name);
        } else {
            LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Opened/created file: %s", file_name);
            file_open = 1;
        }
    }
    HAL_StatusTypeDef tx_status;
    bool file_failure = false;
    switch(lighting_mode){
        case 'd':
            tx_status =  HAL_UART_Transmit(
                UART_camera_port_handle, (uint8_t*)"d", 1, 1000
            );
            if (tx_status != HAL_OK) {
                // Error
                file_failure = true;
                break;
            }
            HAL_Delay(25);
            break;
        case 'm':
            tx_status =  HAL_UART_Transmit(
                UART_camera_port_handle, (uint8_t*)"m", 1, 1000
            );
            if (tx_status != HAL_OK) {
                // Error
                file_failure = true;
                break;
            }
            HAL_Delay(25);
            break;
        case 'n':
            tx_status =  HAL_UART_Transmit(
                UART_camera_port_handle, (uint8_t*)"n", 1, 1000
            );
            if (tx_status != HAL_OK) {
                // Error
                file_failure = true;
                break;
            }
            HAL_Delay(25);
            break;
        case 's':
            tx_status =  HAL_UART_Transmit(
                UART_camera_port_handle, (uint8_t*)"s", 1, 1000
            );
            if (tx_status != HAL_OK) {
                // Error
                file_failure = true;
                break;
            }
            HAL_Delay(25);
            break;
        default:
            return CAM_CAPTURE_STATUS_WRONG_INPUT;
    }

    if (file_failure) {
        // Error in transmition, close file and return error
        if (file_open) {
            const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
            if (close_result < 0)
            {
                LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Error closing file: %s", file_name);
            } else {
                LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "File: %s succesfully closed", file_name);
            }
            file_open = 0;
        }
        return CAM_CAPTURE_STATUS_CAPTURE_FAILURE; // Error
    }

    int capture_code = CAM_receive_image();

    // Turn off the camera.
    const uint8_t eps_off_status = EPS_set_channel_enabled(EPS_CHANNEL_3V3_CAMERA, 0);
    if (eps_off_status != 0) {
        // Continue anyway. Just log a warning.
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Error disabling camera power channel in CAM_Capture_Image: status=%d. Continuing.",
            eps_off_status
        );
    }

    if (capture_code != 0){
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error receiving camera image: Capture Code = %d",
            capture_code
        );
        return CAM_CAPTURE_STATUS_CAPTURE_FAILURE;
    }
    return CAM_CAPTURE_STATUS_TRANSMIT_SUCCESS;
}