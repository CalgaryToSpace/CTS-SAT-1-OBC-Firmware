#ifndef INCLUDE_GUARD__CAMERA_INIT_H
#define INCLUDE_GUARD__CAMERA_INIT_H

#include <stdint.h>
#include <stdbool.h>

#define SENTENCE_LEN    67

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
