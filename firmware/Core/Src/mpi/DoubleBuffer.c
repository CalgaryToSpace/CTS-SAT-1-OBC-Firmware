/*
 * circularBuffer.c
 *
 *  Created on: Mar 16, 2024
 *  Author: Vaibhav Kapoor
 */
#include <DoubleBuffer.h>

// Initialize the Double Buffer
void DoubleBuffer_Init(DoubleBuffer *buffer) {
    buffer->mutex = xSemaphoreCreateMutex();
    buffer->dataReady = xSemaphoreCreateBinary();
    buffer->readIndex = 0;
    buffer->writeIndex = 0;
}


/**
 * Acquire mutex to access the buffer & write received data to the buffer. Once complete release dataReady semaphore to signal new data availability (xSemaphoreGive) & release the mutex.
 * @param buffer - Points to the Double Buffer
 * @param data - Points to the data that needs to be written to the Double Buffer
 */
void DoubleBuffer_Write(DoubleBuffer *buffer, const uint8_t *data) {
    xSemaphoreTake(buffer->mutex, portMAX_DELAY);

    for (uint32_t i = 0; i < sizeof(data); i++) {
        buffer->data[buffer->writeIndex] = data[i];
        buffer->writeIndex = (buffer->writeIndex + 1) % DOUBLE_BUFFER_SIZE;

        // Check for buffer overflow
        if (buffer->writeIndex == buffer->readIndex) {
            buffer->readIndex = (buffer->readIndex + 1) % DOUBLE_BUFFER_SIZE;
        }
    }

    xSemaphoreGive(buffer->mutex);
    xSemaphoreGive(buffer->dataReady);
}

// Reads data from the buffer & return the number of bytes read (up to a maximum length)
uint32_t DoubleBuffer_Read(DoubleBuffer *buffer, uint8_t *data, uint32_t maxLength) {
    if (xSemaphoreTake(buffer->dataReady, 0) == pdFALSE) {
        // No data available
        return 0;
    }

    xSemaphoreTake(buffer->mutex, portMAX_DELAY);

    uint32_t length = 0;
    while (buffer->readIndex != buffer->writeIndex && length < maxLength) {
        data[length] = buffer->data[buffer->readIndex];
        buffer->readIndex = (buffer->readIndex + 1) % DOUBLE_BUFFER_SIZE;
        length++;
    }

    xSemaphoreGive(buffer->mutex);
    return length;
}


//#include <stdio.h>
//#include <stdlib.h>
//#include <pthread.h>
//#include <pthread.h> // Include pthread.h for mutex
//
//// Initialize Circular Buffer
//void CircularBuffer_Init(CircularBuffer_t* buffer) {
//    buffer->data = (uint8_t*)malloc(CIRCULAR_BUFFER_SIZE * sizeof(uint8_t));
//    buffer->head = 0;
//    buffer->tail = 0;
//    buffer->size = CIRCULAR_BUFFER_SIZE;
//    buffer->mutex = xSemaphoreCreateMutex();
//    //pthread_mutex_init(&buffer->mutex, NULL); // Initialize mutex
//}
//
//// Check if buffer is empty
//uint8_t CircularBuffer_IsEmpty(CircularBuffer_t* buffer) {
//    //pthread_mutex_lock(&buffer->mutex);
//	if (xSemaphoreTake(buffer->mutex, portMAX_DELAY) == pdTRUE) {
//		uint8_t isEmpty = (buffer->head == buffer->tail);
//		xSemaphoreGive(buffer->mutex);
//		//pthread_mutex_unlock(&buffer->mutex);
//		return isEmpty;
//	}
//
//    return 0; // Mutex failure
//}
//
//// Check if buffer is full
//uint8_t CircularBuffer_IsFull(CircularBuffer_t* buffer) {
//    //pthread_mutex_lock(&buffer->mutex);
//	if (xSemaphoreTake(buffer->mutex, portMAX_DELAY) == pdTRUE) {
//		uint8_t isFull = ((buffer->head + 1) % buffer->size == buffer->tail);
//		xSemaphoreGive(buffer->mutex);
//		//pthread_mutex_unlock(&buffer->mutex);
//		return isFull;
//	}
//	return 0; // Mutex failure
//}
//
//// Add to circular buffer
//uint8_t CircularBuffer_Enqueue(CircularBuffer_t *buffer, uint8_t *data,
//		uint32_t size) {
//	//pthread_mutex_lock(&buffer->mutex);
//	if (xSemaphoreTake(buffer->mutex, portMAX_DELAY) == pdTRUE) {
//		for (uint32_t i = 0; i < size; i++) {
//			if (((buffer->head + 1) % CIRCULAR_BUFFER_SIZE) != buffer->tail) {
//				buffer->data[buffer->head] = data[i];
//				buffer->head = (buffer->head + 1) % CIRCULAR_BUFFER_SIZE;
//			} else {
//				// Buffer full
//				xSemaphoreGive(buffer->mutex);
//				return 0; // Failed to enqueue
//			}
//		}
//		xSemaphoreGive(buffer->mutex);
//		return 1; // Enqueue successful
//	}
//	return 0; // Mutex failed
//}
//
//// Remove from circular buffer
//uint8_t CircularBuffer_Dequeue(CircularBuffer_t *buffer, uint8_t *data,
//		uint32_t size) {
//	//pthread_mutex_lock(&buffer->mutex);
//	if (xSemaphoreTake(buffer->mutex, portMAX_DELAY) == pdTRUE) {
//		for (uint32_t i = 0; i < size; i++) {
//			if (buffer->head != buffer->tail) {
//				data[i] = buffer->data[buffer->tail];
//				buffer->tail = (buffer->tail + 1) % CIRCULAR_BUFFER_SIZE;
//			} else {
//				// Buffer empty
//				xSemaphoreGive(buffer->mutex);
//				return 0; // Failed to dequeue
//			}
//		}
//		xSemaphoreGive(buffer->mutex);
//		return 1; // Dequeue successful
//	}
//	return 0; // Mutex take failed
//}

