#ifndef INCLUDE_GUARD__CAMERA_INIT_H
#define INCLUDE_GUARD__CAMERA_INIT_H

#include <stdint.h>
#include <stdbool.h>

#define CAM_SENTENCE_LEN    67

uint8_t CAM_change_baudrate(uint32_t bitrate);

uint8_t CAM_setup();
uint8_t CAM_test();



typedef enum CAM_capture_status_enum{
    CAM_CAPTURE_STATUS_TRANSMIT_SUCCESS, CAM_CAPTURE_STATUS_WRONG_INPUT, CAM_CAPTURE_STATUS_CAPTURE_FAILURE} CAM_capture_status_enum;


/**
 * Transmits ASCII telecommand based on input
 * @param lighting - lighting should be a *lower case* char
 * 			d - daylight ambient light
 * 			m - medium ambient light
 * 			n - night ambient light
 * 			s - solar sail contrast and light
 */
enum CAM_capture_status_enum CAM_capture_image(char filename_str[], char lighting_mode);

void CAM_end_camera_receive_due_to_error();

#endif // INCLUDE_GUARD__CAMERA_INIT_H
