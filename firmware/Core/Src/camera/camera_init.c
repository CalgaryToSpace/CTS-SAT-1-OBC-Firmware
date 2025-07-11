#include "debug_tools/debug_uart.h"
#include "camera/camera_init.h"
#include "camera/camera_capture.h"
#include "uart_handler/uart_handler.h"
#include "uart_handler/uart_control.h"
#include "log/log.h"
#include "eps_drivers/eps_channel_control.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "main.h"


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
        return 10; // Invalid baud rate
    }

    const HAL_StatusTypeDef tx_status = HAL_UART_Transmit(
        UART_camera_port_handle, (uint8_t*)set_baudrate_tx_code, 1, 1000
    );
    if (tx_status != HAL_OK) {
        return 11; // Error
    }

    // Wait for UART transmission to complete.
    HAL_Delay(50); // 25 is probably enough.

    return UART_set_baud_rate(UART_camera_port_handle, new_baud_rate);
}

static void camera_clean_up() {
    // Power off the camera.
    const uint8_t eps_status = EPS_set_channel_enabled(EPS_CHANNEL_3V3_CAMERA, 0);
    if (eps_status != 0) {
        // Continue anyway. Just log a warning.
        LOG_message(
            LOG_SYSTEM_EPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Error disabling camera EPS channel: status=%d. Continuing.",
            eps_status
        );
    }
}

/// @brief Set up the camera by powering on and changing the baudrate to 230400.
/// @return 0 on success. The error code from the `CAM_change_baudrate` function, or >100 if an EPS error occurred.
/// @note Does not perform a self-test.
uint8_t CAM_setup() {
    // First, set baudrate to 115200 to reset the camera and OBC UART baud rate.
    // When the camera boots up, it defaults to 115200 baud. In order to command it
    // past that, we need to set the STM32 port's baud rate to 115200 first, then change it to 230400.
    // This is to avoid the camera stalling and being in a weird state where
    // the OBC UART handler's baudrate and the camera's baudrate are different.
    const uint8_t baudrate_reset_status = CAM_change_baudrate(115200);
    if (baudrate_reset_status != 0) {
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error changing camera baudrate to 115200. Error code %d",
            baudrate_reset_status
        );
        return baudrate_reset_status;
    }

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
    const uint8_t baudrate_set_status = CAM_change_baudrate(230400);
    if (baudrate_set_status != 0) {
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error changing camera baudrate: CAM_change_baudrate returned %d",
            baudrate_set_status
        );

        // Turn off camera before exiting
        camera_clean_up();
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
        camera_clean_up();
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
        camera_clean_up();
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
        camera_clean_up();
        return 5;
    }

    // Should contain string "piCAM/FM Test String Transmission".
    if (strstr(UART_test_str, "piCAM/FM Test String Transmission") == NULL) {
        camera_clean_up();
        return 6;
    }

    const char* const test_resp_border_str = "@----------------------------------------------------------------";

    // Ensure string starts with the `test_resp_border_str`.
    if (strncmp(UART_test_str, test_resp_border_str, strlen(test_resp_border_str)) != 0) {
        camera_clean_up();
        return 7;
    }

    // Ensure there's a second border string in the response.
    if (strstr(UART_test_str + strlen(test_resp_border_str), test_resp_border_str) == NULL) {
        camera_clean_up();
        return 8;
    }

    return 0;
}
