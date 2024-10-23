#ifndef __INCLUDE_GUARD__SHA256_H__
#define __INCLUDE_GUARD__SHA256_H__
#include <stdint.h>
#include "debug_tools/debug_uart.h"
#include <cmox_crypto.h>

uint8_t CRYPT_compute_sha256_hash(uint8_t* message, size_t message_length, uint8_t* digest);



#endif /* __INCLUDE_GUARD__SHA256_H__ */
