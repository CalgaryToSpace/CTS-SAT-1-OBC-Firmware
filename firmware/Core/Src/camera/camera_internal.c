#include "littlefs/littlefs_helper.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_driver.h"
#include "debug_tools/debug_uart.h"
#include "camera/camera.h"

//////////////////////////

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "main.h"



// needs to be updated with littleFS
/**
 * @brief creates temporary buffers and receives image data from camera module
 * 
 */
void capture_Image(){
		uint8_t sen[67*300] = {'\0'};
		uint8_t sen1[67*300] = {'\0'};
		uint8_t sen2[67*300] = {'\0'};
		uint8_t sen3[67*300] = {'\0'};
		HAL_UART_Receive(&huart4, sen, 67*300, 10000);
		HAL_UART_Receive(&huart4, sen1, 67*300, 10000);
		HAL_UART_Receive(&huart4, sen2, 67*300, 10000);
		HAL_UART_Receive(&huart4, sen3, 67*300, 10000);


		DEBUG_uart_print_str(sen);
		DEBUG_uart_print_str(sen1);
		DEBUG_uart_print_str(sen2);
		
		//parsing && littleFS
		// LFS_mount(); //UNCOMMENT THESE LINES
		// filename
		char f_n[] = "image";

		if(strlen(sen) != 0){
			//nothing
		} else{
			return 0;
		}

		if(strlen(sen1) != 0){
			// HAL_UART_Transmit(&hlpuart1, (uint8_t*)sen, 67*300, 10000);
			// LFS_write_file(f_n, sen, 67*300); //UNCOMMENT THESE LINES
		} else{
			//trim and transmit 0
			// HAL_UART_Transmit(&hlpuart1, (uint8_t*)sen, strlen(sen), 10000);
			// LFS_write_file(f_n, sen, strlen(sen)); //UNCOMMENT THESE LINES
			return 0;
		}

		if(strlen(sen2) != 0){
			// HAL_UART_Transmit(&hlpuart1, (uint8_t*)sen1, 67*300, 10000);
			// LFS_write_file(f_n, sen1, 67*300); //UNCOMMENT THESE LINES
		} else{
			//trim and send 1
			// HAL_UART_Transmit(&hlpuart1, (uint8_t*)sen1, strlen(sen1), 10000);
			// LFS_write_file(f_n, sen1, strlen(sen1)); //UNCOMMENT THESE LINES
			return 0;
		}

		if(strlen(sen3) != 0){
			// HAL_UART_Transmit(&hlpuart1, (uint8_t*)sen2, 67*300, 10000);
			// LFS_write_file(f_n, sen2, 67*300); //UNCOMMENT THESE LINES
			// trim and send 3
			// HAL_UART_Transmit(&hlpuart1, (uint8_t*)sen3, strlen(sen3), 10000);
			// LFS_write_file(f_n, sen3, strlen(sen3)); //UNCOMMENT THESE LINES
			return 0;
		} else{
			//trim and send 2
			// HAL_UART_Transmit(&hlpuart1, (uint8_t*)sen2, strlen(sen2), 10000);
			// LFS_write_file(f_n, sen2, strlen(sen2)); //UNCOMMENT THESE LINES
			return 0;
		}
		////// TESTING ONLY ///////
		uint8_t readtest[67*600] = {'\0'};
		// LFS_read_file(f_n, 0, readtest, 67*600); //UNCOMMENT THESE LINES
		// LFS_unmount(); //UNCOMMENT THESE LINES
		DEBUG_uart_print_str(readtest);
  }


      /**
     * Transmits ASCII telecommand based on input
     * @param lighting - lighting should be a *lower case* char
     * 			d - daylight ambient light
     * 			m - medium ambient light
     * 			n - night ambient light
     * 			s - solar sail contrast and light
     */
    enum Capture_Status Capture_Image(bool enable_flash, uint8_t lighting_mode){

  	  switch(lighting_mode){
  	  case 'd':
  		  if(enable_flash){
  			  HAL_UART_Transmit(&huart4, (uint8_t*)"D", 1, HAL_MAX_DELAY);
  			  HAL_Delay(25);
  		  }
  		  else{
  			  HAL_UART_Transmit(&huart4, (uint8_t*)"d", 1, HAL_MAX_DELAY);
  			  HAL_Delay(25);
  		  }
  		//   return Transmit_Success;
  	  case 'm':
  	  		  if(enable_flash){
  	  			  HAL_UART_Transmit(&huart4, (uint8_t*)"M", 1, HAL_MAX_DELAY);
  	  			  HAL_Delay(25);
  	  		  }
  	  		  else{
  	  			  HAL_UART_Transmit(&huart4, (uint8_t*)"m", 1, HAL_MAX_DELAY);
  	  			  HAL_Delay(25);
  	  		  }
  	  		// receiveImage();
  	  		// return Transmit_Success;
  	  case 'n':
  	  		  if(enable_flash){
  	  			  HAL_UART_Transmit(&huart4, (uint8_t*)"N", 1, HAL_MAX_DELAY);
  	  			  HAL_Delay(25);
  	  		  }
  	  		  else{
  	  			  HAL_UART_Transmit(&huart4, (uint8_t*)"n", 1, HAL_MAX_DELAY);
  	  			  HAL_Delay(25);
  	  		  }
  	  		// receiveImage();
  	  		// return Transmit_Success;
  	  case 's':
  	  		  if(enable_flash){
  	  			  HAL_UART_Transmit(&huart4, (uint8_t*)"S", 1, HAL_MAX_DELAY);
  	  			  HAL_Delay(25);
  	  		  }
  	  		  else{
  	  			  HAL_UART_Transmit(&huart4, (uint8_t*)"s", 1, HAL_MAX_DELAY);
  	  			  HAL_Delay(25);
  	  		  }
  	  capture_Image();
  	  return Transmit_Success;
  	  default:
  		  return Wrong_input;
  	  }


    }
