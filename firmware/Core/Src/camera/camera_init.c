#include "debug_tools/debug_uart.h"
#include "camera/camera_init.h"
#include "camera/camera_capture.h"
#include "uart_handler/uart_handler.h"
#include "log/log.h"
#include "eps_drivers/eps_channel_control.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "main.h"


/// @brief Set up the camera by powering on and changing the baudrate to 230400.
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
