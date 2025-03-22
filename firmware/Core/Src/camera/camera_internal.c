#include "littlefs/littlefs_helper.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_driver.h"
#include "debug_tools/debug_uart.h"
#include "camera/camera.h"
#include "uart_handler/uart_handler.h"
#include "telecommands/telecommand_args_helpers.h"

//////////////////////////

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "main.h"

/// @brief Timeout duration for receive in milliseconds. Same between bytes and at the start.
static const uint32_t CAMERA_RX_TIMEOUT_DURATION_MS =296000;
static const uint32_t CAMERA_RX_FINISH_DURATION_MS =5000;
 char file_name[] = "image1_0";

/**
 * @brief creates temporary buffers and receives image data from camera module
 * 
 * @return 0: Success, 3: Failed UART reception, 4: Timeout while waiting for first byte
 */
uint8_t CAM_receive_image(){
		// Clear the camera response buffer (Note: Can't use memset because UART_camera_buffer is Volatile)
		for (uint16_t i = 0; i < UART_camera_buffer_len; i++) {
			UART_camera_buffer[i] = 0;
		};
		// clear half bufs
		for (uint16_t i = 0; i < SENTENCE_LEN*23; i++){
			camera_rx_buf[i] = 0;
			camera_rx_half_buf[i] = 0;
		}
		// reset all variables
		camera_write_file = 0;
		camera_write_half_file = 0;
		camera_sentence_counter = 0;
		UART_camera_buffer_write_idx = 0;
		UART_camera_last_write_time_ms = 0;
		

		// Reset UART interrupt buffer write index & record start time for camera response reception
		UART_camera_buffer_write_idx = 0;
		// CAMERA_set_expecting_data(1);
		const uint32_t UART_camera_rx_start_time_ms = HAL_GetTick();
		const uint8_t receive_status = CAMERA_set_expecting_data(1);
		 // Check for UART reception errors
		if (receive_status == 3) {
			// HAL_UART_DMAStop(&huart4);
			CAMERA_set_expecting_data(0);
			return 3; // Error code: Failed UART reception
		}
		LFS_mount();
		while(1){
		// uint32_t camera_current_time = HAL_GetTick();
		// // if camera has finished with image transfer, stop
		// if (camera_current_time - UART_camera_last_write_time_ms > CAMERA_RX_FINISH_DURATION_MS){
		// 	if (UART_camera_last_write_time_ms != 0) {
		// 	camera_current_time = UART_camera_last_write_time_ms + CAMERA_RX_TIMEOUT_DURATION_MS + 1;
		// 	}
		// }

		// change filename every ~600 sentences
		if (camera_sentence_counter >= 590){
			strcpy(file_name, "image1_1");
		}
		if (camera_sentence_counter >= 1190){
			strcpy(file_name, "image1_2");
		}
		// Receive until  response timed out

		// write file after half and complete call backs
		if (camera_write_half_file){
			DEBUG_uart_print_str("in half write file\n");
			UART_camera_last_write_time_ms = HAL_GetTick();
			LFS_append_file(file_name, camera_rx_half_buf, SENTENCE_LEN*23);
			camera_write_half_file = 0;
		}
		if (camera_write_file){
			DEBUG_uart_print_str("in write file\n");
			UART_camera_last_write_time_ms = HAL_GetTick();
			LFS_append_file(file_name, camera_rx_buf, SENTENCE_LEN*23);
			camera_write_file = 0;
		}
        // Timeout before receiving the first byte from the MPI
        if (UART_camera_buffer_write_idx == 0) {
			// if write idx = 0, and timeout occurs
            if((HAL_GetTick() - UART_camera_rx_start_time_ms) > CAMERA_RX_TIMEOUT_DURATION_MS) {
                // if last write time = 0, error 4
				if (UART_camera_last_write_time_ms == 0){
					CAMERA_set_expecting_data(0);
					return 4; // Error code: Timeout waiting for 1st byte
				}
				else{
					// otherwise there may be data in first half of buffer, copy it and set write to 1
					// Copy the buffer to the last received byte index & clear the UART buffer
					for (uint16_t i = 0; i < UART_camera_buffer_len/2; i++) {
						camera_rx_half_buf[i] = UART_camera_buffer[i];
						UART_camera_buffer[i] = 0;
					}
					DEBUG_uart_print_str("timeout write file 1\n"); 
					camera_write_half_file = 1;
					CAMERA_set_expecting_data(0);
					// break out of while loop
					break;
				}
            }            
        }

        // Timeout in between (or end of) receiving bytes from the MPI
		// if write idx is not 0 then it must be 1
        else {
			// if write idx is 1, there may be data in second half of buffer, write it
            const uint32_t current_time = HAL_GetTick(); // Get current time
             if (
                (current_time > UART_camera_last_write_time_ms) // Important seemingly-obvious safety check.
                && ((current_time - UART_camera_last_write_time_ms) > CAMERA_RX_TIMEOUT_DURATION_MS)
            ) {

				// data in second half of uart camera buffer
				// Copy the buffer to the last received byte index & clear the UART buffer
				for (uint16_t i = UART_camera_buffer_len/2; i < UART_camera_buffer_len; i++) {
					camera_rx_buf[i-UART_camera_buffer_len/2] = UART_camera_buffer[i];
					UART_camera_buffer[i] = 0;
				}
				DEBUG_uart_print_str("timeout write file 2\n"); 
				camera_write_file = 1;
				CAMERA_set_expecting_data(0);
				// break out of while loop
                break;
            }
        }
		}
		LFS_unmount();
		// Outside of while loop
		// if write file is 1 then write to memory before returning 0
		if (camera_write_half_file){
			UART_camera_last_write_time_ms = HAL_GetTick();
			LFS_mount();
			LFS_append_file(file_name, camera_rx_half_buf, SENTENCE_LEN*23);
			LFS_unmount();
			camera_write_half_file = 0;
		}
		if (camera_write_file){
			UART_camera_last_write_time_ms = HAL_GetTick();
			LFS_mount();
			LFS_append_file(file_name, camera_rx_buf, SENTENCE_LEN*23);
			LFS_unmount();
			camera_write_file = 0;
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
	  ////////////////// TESTING ////////
	  uint8_t read_buf[SENTENCE_LEN*50];
	  LFS_mount();
	  for(int i = 0; i <= 12; i++){
	  LFS_read_file("image1_0", SENTENCE_LEN*(50*i), read_buf, SENTENCE_LEN*50);
	  DEBUG_uart_print_str((char *) read_buf);
	}
	LFS_unmount();
	memset(read_buf, '\0', strlen(read_buf));
	LFS_mount();
	for(int i = 0; i <= 12; i++){
		LFS_read_file("image1_1", SENTENCE_LEN*(50*i), read_buf, SENTENCE_LEN*50);
		DEBUG_uart_print_str((char *) read_buf);
	}
	  LFS_unmount();
	  memset(read_buf, '\0', strlen(read_buf));
	  LFS_mount();
	for(int i = 0; i <= 12; i++){
		LFS_read_file("image1_2", SENTENCE_LEN*(50*i), read_buf, SENTENCE_LEN*50);
		DEBUG_uart_print_str((char *) read_buf);
	}
	  LFS_unmount();


	  ////////////////// END TESTING ////
	  if (capture_code != 0){
		char str[50];
		snprintf(str, 28, "Error Capturing image: %d\n", capture_code);
	  	DEBUG_uart_print_str(str);
		return Capture_Failure;
	  }
	  DEBUG_uart_print_str("Exited capture_image, going to return out of telecommand\n");
  	  return Transmit_Success;


    }
