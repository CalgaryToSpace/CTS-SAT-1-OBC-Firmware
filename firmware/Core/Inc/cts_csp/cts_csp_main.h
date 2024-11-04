#ifndef INCLUDE_GUARD__CTS_CSP_MAIN_H
#define INCLUDE_GUARD__CTS_CSP_MAIN_H //from parkers branch
// https://docs.kubos.com/1.1.0/apis/libcsp/i2cdriver.html

#include <csp/interfaces/csp_if_i2c.h> //my addition
#include <stdint.h>
#define E_NO_ERROR -1 //return value of driver, should be -1 if successful
#define I2C_MTU //MAX transfer length on i2c
#define I2C_MASTER
#define I2C_SLAVE //i2c device modes
typedef struct i2c_frame_s i2c_frame_t; //website: "Data structure for I2C frames."
/**
INCLUDE PARAMS HERE FOR I2C_CALLBACK
+WHATEVER EXPLANATIONS
 */
typedef void (*i2c_callback_t)(i2c_frame_t *frame, void *pxTaskWoken); //website "Initialise the I2C driver."
/**
INCLUDE PARAMS HERE FOR INIT
+WHATEVER EXPLANATIONS
 */
void CSP_init_for_cts1(int handle, int mode, uint8_t addr, uint16_t speed, 
int queue_len_tx, int queue_len_rx, i2c_callback_t callback); //from website

int CSP_send_for_cts1(int handle, i2c_frame_t *frame, uint16_t timeout); //from website


uint8_t CSP_demo_1(); // from parker's branch

#endif // INCLUDE_GUARD__CTS_CSP_MAIN_H
