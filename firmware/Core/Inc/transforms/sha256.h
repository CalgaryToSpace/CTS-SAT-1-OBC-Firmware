#include <stdint.h>
#include "cmox_crypto.h"
#include "debug_tools/debug_uart.h"

uint8_t compute_sha256_hash(uint8_t* message, size_t message_length, uint8_t* digest);
