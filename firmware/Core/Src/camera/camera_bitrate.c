
#include "debug_tools/debug_uart.h"
#include "camera/camera_bitrate.h"
#include "uart_handler/uart_handler.h"

//////////////////////////

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "main.h"

/**
 * @brief sends telecommand to camera to change baudrate of camera. DO NOT USE THIS WITHOUT ALSO CHANGING THE BAUDRATE OF THE CAMERA UART PORT
 * @param bitrate 0 through 9 to set bitrate of camera
 * @return 0: Success, 1: Invalid input
 */
uint8_t CAM_change_baudrate(char bitrate){
    if (bitrate < '0' || bitrate > '9') {
        return 1;  // Invalid input
    }
    uint8_t baudrate_str[1] = {bitrate};
    HAL_UART_Transmit(&huart4, baudrate_str, 1, 1500);
    return 0;
}

/**
 * @brief sends telecommand to camera to change baudrate of camera. DO NOT USE THIS WITHOUT ALSO CHANGING THE BAUDRATE OF THE CAMERA UART PORT
 * @return 0: Success, 1: Error receiving (probably timeout), 2: Invalid bitrate 
 */
uint8_t CAM_setup(){
    // change baudrate to 2400 on camera
    uint8_t bitrate_status = CAM_change_baudrate('1');
    if (bitrate_status == 1){ return 2;}
    HAL_Delay(25);
    // deinit camera uart port and reinitialize with 9600 baudrate
    HAL_UART_DeInit(&huart4);
    huart4.Init.BaudRate = 2400;
    HAL_UART_Init(&huart4);
    // print test string to ensure it worked
    uint8_t  UART_test_sen[TEST_SEN_LEN];
    UART_test_sen[0] = 0;
    HAL_UART_Transmit(&huart4, (uint8_t*)"t", 1, 1500);
    const HAL_StatusTypeDef rec_stat = HAL_UART_Receive(&huart4, (uint8_t*) &UART_test_sen, TEST_SEN_LEN, 5000);
    if (rec_stat != HAL_OK){ return 1;}
    DEBUG_uart_print_str((char *) UART_test_sen);
    // if UART_test_sen is populated success, otherwise error
    if (UART_test_sen[0] == 0){ return 1;}
    return 0;
}

/**
 * @brief runs test telecommand and receives status message
 * @return 0: Success, 1: Error
 */
uint8_t CAM_test(){
    // print test string 
    uint8_t  UART_test_str[TEST_SEN_LEN];
    UART_test_str[0] = 0;
    HAL_UART_Transmit(&huart4, (uint8_t*)"t", 1, HAL_MAX_DELAY);
    HAL_UART_Receive(&huart4, (uint8_t*) &UART_test_str, TEST_SEN_LEN, 5000);
    DEBUG_uart_print_str((char *) UART_test_str);
    // if UART_test_sen is populated success, otherwise error
    if (UART_test_str[0] == 0){
        UART_test_str[0] = 0;
        return 1;
    }
    UART_test_str[0] = 0;
    return 0;
}