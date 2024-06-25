/*
 * frameBuffer.h
 *
 *  Created on: Mar 16, 2024
 *  Author: Vaibhav Kapoor
 */

#ifndef __INCLUDE_GUARD__DOUBLE_BUFFER_H__
#define __INCLUDE_GUARD__DOUBLE_BUFFER_H__

#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"

#define DOUBLE_BUFFER_SIZE 1024

typedef struct
{
    uint8_t data[DOUBLE_BUFFER_SIZE]; // Data that is being stored into the buffer
    SemaphoreHandle_t mutex;          // Used to make sure only one thread can access the buffer at a time
    SemaphoreHandle_t dataReady;      // Used to signal if new data is available in the buffer
    uint32_t readIndex;               // Tracks current read position in the buffer
    uint32_t writeIndex;              // Tracks current write position in the buffer
} DoubleBuffer;

void DoubleBuffer_Init(DoubleBuffer *buffer);
void DoubleBuffer_Write(DoubleBuffer *buffer, const uint8_t *data);
uint32_t DoubleBuffer_Read(DoubleBuffer *buffer, uint8_t *data, uint32_t maxLength);

#endif /* __INCLUDE_GUARD__DOUBLE_BUFFER_H__ */
