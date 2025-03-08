#include "littlefs/littlefs_helper.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_driver.h"
#include "debug_tools/debug_uart.h"
#include "camera/camera.h"
#include "uart_handler/uart_handler.h"

//////////////////////////

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "main.h"

/// @brief Timeout duration for receive in milliseconds. Same between bytes and at the start.
static const uint16_t CAMERA_RX_TIMEOUT_DURATION_MS = 20000;

// uint8_t sen[67*300] = {'\0'};
// uint8_t sen1[67*300] = {'\0'};
// uint8_t sen2[67*300] = {'\0'};
// uint8_t sen3[67*300] = {'\0'};
// uint8_t camera_rx_buf[SENTENCE_LEN*125] = {'\0'};
// UART_HandleTypeDef *UART_camera_port_handle = &huart4;



// needs to be updated with littleFS
/**
 * @brief creates temporary buffers and receives image data from camera module
 * 
 * @return 0: Success, 3: Failed UART reception, 4: Timeout while waiting for first byte
 */
uint8_t CAM_receive_image(){
		// Clear the camera response buffer (Note: Can't use memset because UART_camera_buffer is Volatile)
		for (uint16_t i = 0; i < UART_camera_buffer_len; i++) {
			UART_camera_buffer[i] = 0;
		}

		// Reset UART interrupt buffer write index & record start time for camera response reception
		UART_camera_buffer_write_idx = 0;
		// CAMERA_set_expecting_data(1);
		const uint32_t UART_camera_rx_start_time_ms = HAL_GetTick();
		const uint8_t receive_status = CAMERA_set_expecting_data(1);
		 // Check for UART reception errors
		if (receive_status == 3) {
			// HAL_UART_DMAStop(&huart4);
			CAMERA_set_expecting_data(0);
			// TODO set error code
			return 3; // Error code: Failed UART reception
		}
		while(1){
		// Receive until  response timed out

        // Timeout before receiving the first byte from the MPI
        if (UART_camera_buffer_write_idx == 0) {
			// DEBUG_uart_print_str("Write IDX == 0\n");
            if((HAL_GetTick() - UART_camera_rx_start_time_ms) > CAMERA_RX_TIMEOUT_DURATION_MS) {
                // Stop reception from the MPI & Reset mpi UART & transceiver mode states
                // HAL_UART_DMAStop(UART_camera_port_handle);
				CAMERA_set_expecting_data(0);
				// TODO ERROR CODES
                return 4; // Error code: Timeout waiting for 1st byte
            }            
        }

        // Timeout in between (or end of) receiving bytes from the MPI
        else {
            const uint32_t current_time = HAL_GetTick(); // Get current time
             if (
                (current_time > UART_camera_last_write_time_ms) // Important seemingly-obvious safety check.
                && ((current_time - UART_camera_last_write_time_ms) > CAMERA_RX_TIMEOUT_DURATION_MS)
            ) {
                // data in first half of camera uart buffer
        // if (UART_camera_buffer_write_idx <= UART_camera_buffer_len/2) {
            // Copy the buffer to the last received byte index & clear the UART buffer
			for (uint16_t i = 0; i < UART_camera_buffer_len/2; i++) {
				camera_rx_buf[i] = UART_camera_buffer[i];
				UART_camera_buffer[i] = 0;
			}
            DEBUG_uart_print_str("timeout write file 1\n"); 
			camera_write_file = 1;
			CAMERA_set_expecting_data(0);
        // }

		// data in second half of uart camera buffer
        // else {
			UART_camera_buffer_write_idx = 0;
            // Copy the buffer to the last received byte index & clear the UART buffer
			for (uint16_t i = UART_camera_buffer_len/2; i < UART_camera_buffer_len; i++) {
				camera_rx_buf[i-UART_camera_buffer_len/2] = UART_camera_buffer[i];
				UART_camera_buffer[i] = 0;
			}
            DEBUG_uart_print_str("timeout write file 2\n"); 
			camera_write_file = 1;
			CAMERA_set_expecting_data(0);
        // }
                break;
            }
        }
			// CAMERA_set_expecting_data(1);
		}
		
		return 0;
  }


      /**
     * Transmits ASCII telecommand based on input
     * @param lighting - lighting should be a *lower case* char
     * 			d - daylight ambient light
     * 			m - medium ambient light
     * 			n - night ambient light
     * 			s - solar sail contrast and light
	 * @return Transmit_Success: Successfully captured image, Wrong_input: invalid parameter input, Capture_Failure: Error in image reception
     */
    enum Capture_Status CAM_Capture_Image(bool enable_flash, uint8_t lighting_mode){

  	  switch(lighting_mode){
  	  case 'd':
			HAL_UART_Transmit(&huart4, (uint8_t*)"d", 1, HAL_MAX_DELAY);
			HAL_Delay(25);
			break;
  		  
  	  case 'm':
			HAL_UART_Transmit(&huart4, (uint8_t*)"m", 1, HAL_MAX_DELAY);
			HAL_Delay(25);
			break;
  	  		  
  	  case 'n':
			HAL_UART_Transmit(&huart4, (uint8_t*)"n", 1, HAL_MAX_DELAY);
			HAL_Delay(25);
			break;
  	  		  
  	  case 's':
			HAL_UART_Transmit(&huart4, (uint8_t*)"s", 1, HAL_MAX_DELAY);
			HAL_Delay(25);
			break;
  	  		  
  	  default:
  		  return Wrong_input;
  	  }
  	  int capture_code = CAM_receive_image();
	  if (capture_code != 0){
		char str[50];
		snprintf(str, 28, "Error Capturing image: %d\n", capture_code);
	  	DEBUG_uart_print_str(str);
		return Capture_Failure;
	  }
	  DEBUG_uart_print_str("Exited capture_image, going to return out of telecommand\n");
  	  return Transmit_Success;


    }
