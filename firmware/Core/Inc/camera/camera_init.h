#ifndef INCLUDE_GUARD__CAMERA_INIT_H
#define INCLUDE_GUARD__CAMERA_INIT_H

#include <stdint.h>
#include <stdbool.h>
#include "littlefs/lfs.h"

#define SENTENCE_LEN    67
/// @brief Timeout duration for camera receive in milliseconds
static const uint32_t CAMERA_RX_TIMEOUT_DURATION_MS =12000;

/// @brief Global variables for file and file_open 
extern uint8_t file_open;
extern lfs_file_t file;
extern char *file_name;
extern uint32_t UART_camera_rx_start_time_ms;
extern bool cam_receive_error;

uint8_t CAM_change_baudrate(uint32_t bitrate);

uint8_t CAM_setup();
uint8_t CAM_test();


typedef enum Capture_Status_enum{Transmit_Success, Wrong_input, Capture_Failure} Capture_Status_enum;



/**
 * Transmits ASCII telecommand based on input
 * @param lighting - lighting should be a *lower case* char
 * 			d - daylight ambient light
 * 			m - medium ambient light
 * 			n - night ambient light
 * 			s - solar sail contrast and light
 */
Capture_Status_enum CAM_Capture_Image(bool enable_flash, uint8_t lighting_mode);

#endif // INCLUDE_GUARD__CAMERA_INIT_H
