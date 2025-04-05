#ifndef INCLUDE_GUARD__CAMERA_INIT_H
#define INCLUDE_GUARD__CAMERA_INIT_H

#include <stdint.h>

uint8_t CAM_change_baudrate(uint32_t bitrate);

uint8_t CAM_setup();
uint8_t CAM_test();

#endif // INCLUDE_GUARD__CAMERA_INIT_H
