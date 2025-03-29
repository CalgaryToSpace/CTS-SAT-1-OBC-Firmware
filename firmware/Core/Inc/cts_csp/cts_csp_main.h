#ifndef INCLUDE_GUARD__CTS_CSP_MAIN_H
#define INCLUDE_GUARD__CTS_CSP_MAIN_H

#include <stdint.h>

#include "csp/interfaces/csp_if_i2c.h"

void CSP_init_for_cts1();

uint8_t CSP_demo_1();

int CSP_i2c_driver_tx(void * driver_data, csp_i2c_frame_t * packet);


#endif // INCLUDE_GUARD__CTS_CSP_MAIN_H
