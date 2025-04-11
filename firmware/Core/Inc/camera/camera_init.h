#ifndef INCLUDE_GUARD__CAMERA_INIT_H
#define INCLUDE_GUARD__CAMERA_INIT_H

#include <stdint.h>
#include <stdbool.h>

#define SENTENCE_LEN    67
/// @brief Timeout duration for camera receive in milliseconds
static const uint32_t CAMERA_RX_TIMEOUT_DURATION_MS =12000;

/// @brief Global variables for file and file_open 
uint8_t file_open = 0;
lfs_file_t file;
char file_name[] = "image1_0";
uint32_t UART_camera_rx_start_time_ms;
bool cam_receive_error;

uint8_t CAM_change_baudrate(uint32_t bitrate);

uint8_t CAM_setup();
uint8_t CAM_test();


enum Capture_Status{Transmit_Success, Wrong_input, Capture_Failure};


/**
 * Transmits ASCII telecommand based on input
 * @param lighting - lighting should be a *lower case* char
 * 			d - daylight ambient light
 * 			m - medium ambient light
 * 			n - night ambient light
 * 			s - solar sail contrast and light
 */
enum Capture_Status CAM_Capture_Image(bool enable_flash, uint8_t lighting_mode);

#endif // INCLUDE_GUARD__CAMERA_INIT_H
