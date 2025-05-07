#ifndef INCLUDE_GUARD__CAMERA_COMMANDS_H
#define INCLUDE_GUARD__CAMERA_COMMANDS_H

#include <stdint.h>
#include <stdbool.h>

#define CAM_SENTENCE_LEN    67

// Note: 23 sentences is too few. 100 sometimes is too few. 250 is a good number, I think.
#define CAM_SENTENCES_PER_HALF_CALLBACK 250

uint8_t CAM_change_baudrate(uint32_t bitrate);

uint8_t CAM_setup();
uint8_t CAM_test();
void CAM_repeated_error_log_message();


typedef enum CAM_capture_status_enum {
    CAM_CAPTURE_STATUS_TRANSMIT_SUCCESS,
    CAM_CAPTURE_STATUS_WRONG_INPUT,
    CAM_CAPTURE_STATUS_LFS_NOT_MOUNTED,
    CAM_CAPTURE_STATUS_LFS_FAILED_OPENING_CREATING_FILE,
    CAM_CAPTURE_STATUS_LFS_FAILED_WRITING_HEADER,
    CAM_CAPTURE_STATUS_FAILED_TRANSMITTING_LIGHTING_MODE,
    CAM_CAPTURE_STATUS_LFS_FAILED_CLOSING_FILE,
    CAM_CAPTURE_STATUS_CAPTURE_FAILURE,
} CAM_capture_status_enum;


/**
 * Transmits ASCII telecommand based on input
 * @param lighting - lighting should be a *lower case* char
 * 			d - daylight ambient light
 * 			m - medium ambient light
 * 			n - night ambient light
 * 			s - solar sail contrast and light
 */
CAM_capture_status_enum CAM_capture_image(char filename_str[], char lighting_mode);

#endif // INCLUDE_GUARD__CAMERA_CAMERA_COMMANDS_H
