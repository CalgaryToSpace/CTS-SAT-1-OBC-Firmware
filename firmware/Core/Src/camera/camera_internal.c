#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "main.h"

enum Capture_Status{Transmit_Success, Wrong_input};

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
  	  rec2();
  	  return Transmit_Success;
  	  default:
  		  return Wrong_input;
  	  }


    }